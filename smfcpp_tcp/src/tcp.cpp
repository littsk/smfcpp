#include "tcp.hpp"

#include <iostream>
#include <exception>

#include <string.h>
#include <fcntl.h>

#include <cstring>

namespace smfcpp
{
namespace tcp
{

bool 
recvn(const int sockfd, uint8_t * buf, const size_t n, int flags)
{
    if (sockfd == -1) return false;
    int idx = 0, nleft = n, nread = 0;
    while(nleft){
        nread = recv(sockfd, buf + idx, nleft, flags);
        if(nread <= 0){
            perror("recv");
            return false;
        }
        idx += nread;
        nleft -= nread;
    }
    return true;
}

bool 
sendn(const int sockfd, const uint8_t * buf, const size_t n, int flags)
{   
    if (sockfd == -1) return false;
    int idx = 0, nwrite = 0, nleft = n;
    while(nleft){
        nwrite = send(sockfd, buf + idx, nleft, flags);
        if(nwrite <= 0){
            perror("send");
            return false;
        }
        idx += nwrite;
        nleft -= nwrite;
    }
    return true;
}



/**
 * PackHead realization
 */
PackHead::PackHead(PackType type, uint32_t len)
: m_type(type),
  m_len(len)
{}

PackType PackHead::get_type()
{
    return m_type;
}

uint32_t PackHead::get_len()
{
    return m_len;
}

bool PackHead::send_to(const int sockfd){
    PackHead net_head(
        (PackType)htonl((uint32_t)m_type),
        htonl(m_len)
    );
    bool res = sendn(sockfd, (uint8_t *)&net_head, sizeof(PackHead), 0);
    return res;
}

bool PackHead::recv_from(const int sockfd){
    PackHead head;
    bool res = recvn(sockfd, (uint8_t *)&head, sizeof(PackHead), 0);
    if(res == false){
        return false;
    }
    m_type = (PackType)ntohl((uint32_t)head.get_type());
    m_len = ntohl(head.get_len());
    return true;
}


/**
 * Client realization
 */

Client::Client(
    const std::string & name, 
    const std::string & ip_address, 
    const int port,
    const int timeout,
    const int max_reconnects
    )
: Node(name),
  m_ip_address(ip_address),
  m_port(port),
  m_is_connected(false),
  m_max_reconnects(max_reconnects)
{
    std::memset(&m_server_address, 0, sizeof(m_server_address));
    m_server_address.sin_family = AF_INET;
    m_server_address.sin_port = htons(port);
    if(inet_pton(AF_INET, m_ip_address.c_str(), &m_server_address.sin_addr) <= 0){
        perror("inet_pton");
        std::cerr << "client: Invalid IP address" << std::endl;
    }

    m_timeout.tv_sec = timeout;
    m_timeout.tv_usec = 0;
}

Client::~Client()
{
    close(m_socket_fd);
}

bool
Client::connect_to_server()
{
    if(m_is_connected){
        return true;
    }
    m_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(m_socket_fd < 0){
        perror("client: socket");
        return false;
    }
    if(connect(m_socket_fd, 
        (struct sockaddr *)&m_server_address, 
        sizeof(m_server_address)) < 0){
        
        perror("client: connect");
        return false;
    }
    std::cout << "client: connect to server" << std::endl;
    
    m_is_connected = true;
    m_hearts = MAX_HEARTS;
    FD_ZERO(&m_readfds);
    FD_SET(m_socket_fd, &m_readfds);

    // if it is the first time to connect, then it need set the recver and heart_timer
    if(m_recver == nullptr){
        auto check_func = [=](){
            fd_set tmpfds = this->m_readfds;
            timeval tmp_timeout = this->m_timeout;
            int res = select(this->m_socket_fd + 1, &tmpfds, NULL, NULL, &tmp_timeout);
            if(res > 0){
                return true;
            }
            else if(res < 0){
                perror("client: select");
                return false;
            }
            else{
                // printf("client: select timeout\n");
                return false;
            }
        };

        auto callback = [=](){
            PackHead head;
            if(head.recv_from(this->m_socket_fd) == false){
                this->m_is_connected = false;
                printf("client: connection losed\n");
                return;
            }

            if(head.get_type() == PackType::heart){
                this->m_hearts.store(MAX_HEARTS);
                printf("client: heart recieved\n");
            }
            else if(head.get_type() == PackType::data){
                std::vector<uint8_t> data;
                data.resize(head.get_len());
                if(this->recv_data(data) == false){
                    this->m_is_connected = false;
                    printf("client: connection losed\n");
                    return;
                }
                recv_callback(data);
            }
            else{
                std::cerr << "client: unknow recive error!" << std::endl;
            }

            return;
        };
        m_recver = this->create_recver(check_func, callback);
    }

    if(m_heart_timer == nullptr){
        m_heart_timer = this->create_timer(
            std::chrono::seconds(HEART_BEATS_INTERVAL), 
            std::bind(&Client::send_heart, this));
    }

    if(m_keep_alive_timer == nullptr){
        m_keep_alive_timer = this->create_timer(
            std::chrono::seconds(HEART_BEATS_INTERVAL),
            std::bind(&Client::keep_alive, this)
        );
    }
    
    return true;
}

bool 
Client::connect_ensure()
{
    if(m_is_connected){
        return true;
    }

    for(int i = 0; i < m_max_reconnects; ++i){
        printf("start reconnect: %d\n", i);
        // if file haven't been close, close it
        int flags = fcntl(m_socket_fd, F_GETFL);
        if(flags != -1){
            close(m_socket_fd);
        }

        if(connect_to_server() == true){
            return true;
        }

        // wait for some time to reconnect
        sleep(HEART_BEATS_INTERVAL);
    }

    return false;
}

bool 
Client::send_data(const std::vector<uint8_t> & data)
{
    std::lock_guard lock(m_mutex);

    if(!connect_ensure()){
        std::cerr << "client: server is not connected" << std::endl;
        return false;
    }

    PackHead head(PackType::data, data.size());
    if(head.send_to(m_socket_fd) == false){
        m_is_connected = false;
        printf("client: connection losed\n");
        return false;
    }

    bool res = sendn(m_socket_fd, data.data(), data.size(), 0);
    if(res == false){
        m_is_connected = false;
        printf("client: connection losed\n");
        return false;
        // std::string error_message(256, 0);
        // strerror_r(errno, error_message.data(), error_message.size());
        // throw std::runtime_error("send: " + error_message);
    }

    printf("client: data send success\n");
    return true;
}

bool 
Client::recv_data(std::vector<uint8_t> & data)
{
    std::lock_guard lock(m_mutex);

    if(!connect_ensure()){
        std::cerr << "server is not connected" << std::endl;
        return false;
    }

    bool res = recvn(m_socket_fd, data.data(), data.size(), 0);
    if(res == false){
        m_is_connected = false;
        printf("client: connection losed\n");
        return false;
    }

    printf("client: recieved data success\n");
    return true;
}

void
Client::recv_callback(std::vector<uint8_t> & data)
{   
    printf("client: recived data: ");
    for(int i = 0; i < data.size(); ++i){
        printf("%c", data[i]);
    }
    printf("\n");
}

bool
Client::send_heart()
{
    std::lock_guard lock(m_mutex);
    
    if(!connect_ensure()){
        std::cerr << "client: server is not connected" << std::endl;
        return false;
    }

    PackHead head(PackType::heart, 0);
    if(head.send_to(m_socket_fd) == false){
        m_is_connected = false;
        printf("client: connection losed\n");
        return false;
    }

    printf("client: heart send success\n");
    return true;
}


void 
Client::keep_alive(){
    if(m_hearts.load() == 0){
        m_is_connected = false;
        printf("client: connection losed\n");
    }
    else{
        m_hearts.store(m_hearts.load() - 1);
    }
}


/**
 * Server realization
 */

Server::Server(const std::string & name, int port)
: Node(name),
  m_port(port)
{
    if((m_listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("server: socket");
        return;
    }

    std::memset(&m_server_address, 0, sizeof(m_server_address));
    m_server_address.sin_family = AF_INET;
    m_server_address.sin_addr.s_addr = INADDR_ANY;
    m_server_address.sin_port = htons(port);

    if(bind(m_listen_fd, 
        (struct sockaddr*)&m_server_address, 
        sizeof(m_server_address)) < 0)
    {
        close(m_listen_fd);
        perror("server: bind");
        return;
    }

    if(listen(m_listen_fd, 3)){
        close(m_listen_fd);
        perror("server: listen");
        return;
    }

    FD_ZERO(&m_readfds);
    FD_SET(m_listen_fd, &m_readfds);
    m_max_fd = m_listen_fd;
}


void Server::run()
{
    auto check_func = [this](){
        // printf("server: check start, using select: max fd number is %d\n", this->m_max_fd);
        this->m_tmpfds = this->m_readfds;
        if(select(this->m_max_fd + 1, &this->m_tmpfds, NULL, NULL, NULL) > 0){
            return true;
        }
        else{
            perror("server: select");
            return false;
        }
        
    };

    auto callback = [this](){
        printf("server: execute callback\n");
        for(int event_fd = 0; event_fd <= this->m_max_fd; ++event_fd){
            if(FD_ISSET(event_fd, &this->m_tmpfds) <= 0){
                continue;
            }
            else if(event_fd == this->m_listen_fd){
                struct sockaddr_in client_addr;
                socklen_t socket_addr_len = sizeof(client_addr);
                int client_socket_fd = accept(this->m_listen_fd, 
                    (sockaddr *)&client_addr, 
                    &socket_addr_len);

                if(client_socket_fd < 0){
                    perror("server: accept");
                }
                printf("server: %s: %d connected\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);

                FD_SET(client_socket_fd, &this->m_readfds);
                if(this->m_max_fd < client_socket_fd){
                    this->m_max_fd = client_socket_fd;
                }
                this->m_hearts_mp[client_socket_fd].store(MAX_HEARTS);
                this->m_mutex_mp[client_socket_fd];
                continue;
            }
            else{
                PackHead head;
                if(head.recv_from(event_fd) == false){
                    losed_clinet_handle(event_fd);
                    continue;
                }

                if(head.get_type() == PackType::heart){
                    this->m_hearts_mp[event_fd].store(MAX_HEARTS);
                    printf("server: recieved heart from client_socket_fd: %d\n", event_fd);
                }
                else if(head.get_type() == PackType::data){
                    std::vector<uint8_t> data;
                    data.resize(head.get_len());
                    if(this->recv_data(event_fd, data) == false){
                        continue;
                    }
                    recv_callback(data);
                }
                else{
                    std::cout << "server: unknow recive error! colse client connect" << std::endl;
                    losed_clinet_handle(event_fd);
                }
            }
        }
    };

    m_recver = this->create_recver(check_func, callback);

    m_heart_timer = this->create_timer(
        std::chrono::seconds(HEART_BEATS_INTERVAL), 
        std::bind(&Server::send_heart, this));

    m_keep_alive_timer = this->create_timer(
        std::chrono::seconds(HEART_BEATS_INTERVAL),
        std::bind(&Server::keep_alive, this));

}

void Server::losed_clinet_handle(int client_socket_fd)
{
    printf("server: clinet_socket_fd: %d losed\n", client_socket_fd);
    close(client_socket_fd);
    FD_CLR(client_socket_fd, &this->m_readfds);
    if(client_socket_fd == this->m_max_fd){
        for(int i = this->m_max_fd; i >= 0; --i){
            if(FD_ISSET(i, &this->m_readfds)){
                this->m_max_fd = i;
            }
        }
    }
    this->m_hearts_mp.erase(client_socket_fd);
    this->m_mutex_mp.erase(client_socket_fd);
}

bool 
Server::send_data(int client_socket_fd, const std::vector<uint8_t> & data)
{
    std::lock_guard lock(this->m_mutex_mp[client_socket_fd]);

    PackHead head(PackType::data, data.size());
    if(head.send_to(client_socket_fd) == false){
        return false;
    }

    bool res = sendn(client_socket_fd, data.data(), data.size(), 0);
    if(res == false){
        losed_clinet_handle(client_socket_fd);
        return false;
    }
    return true;
}

bool 
Server::recv_data(int client_socket_fd, std::vector<uint8_t> & data)
{
    std::lock_guard lock(this->m_mutex_mp[client_socket_fd]);

    bool res = recvn(client_socket_fd, data.data(), data.size(), 0);
    if(res == false){
        losed_clinet_handle(client_socket_fd);
        return false;
    }

    return true;
}

void
Server::recv_callback(std::vector<uint8_t> & data)
{   
    printf("server: recived data: ");
    for(int i = 0; i < data.size(); ++i){
        printf("%c", data[i]);
    }
    printf("\n");
}

void Server::send_heart()
{
    for(int client_fd = 0; client_fd <= m_max_fd; ++client_fd){
        if(FD_ISSET(client_fd, &m_readfds) && client_fd != m_listen_fd){
            if(send_heart_once(client_fd)){
                printf("server: send heart to client_socket_fd: %d success\n", client_fd);
            }
            else{
                printf("server: send heart to client_socket_fd: %d failed\n", client_fd);
            }
        }
    }
}

bool Server::send_heart_once(int client_socket_fd)
{   
    std::lock_guard<std::mutex> lock(m_mutex_mp[client_socket_fd]);

    PackHead head(PackType::heart, 0);
    if(head.send_to(client_socket_fd) == false){
        losed_clinet_handle(client_socket_fd);
        return false;
    }
    return true;
}

void Server::keep_alive()
{
    printf("server: alive_check\n");
    for(int client_fd = 0; client_fd <= m_max_fd; ++client_fd){
        if(FD_ISSET(client_fd, &m_readfds) && client_fd != m_listen_fd){
            printf("socket %d heart: %d\n", client_fd, m_hearts_mp[client_fd].load());
            if(m_hearts_mp[client_fd].load() == 0){
                losed_clinet_handle(client_fd);
            }
            else{
                m_hearts_mp[client_fd].store(m_hearts_mp[client_fd].load() - 1);
            }
        }
    }
}

}
}


