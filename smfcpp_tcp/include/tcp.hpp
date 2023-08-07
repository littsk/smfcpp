#ifndef SMFCPP__TCP_HPP_
#define SMFCPP__TCP_HPP_

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <string>

#include <node.hpp>
#include <macros.hpp>

#define MAX_RECONNECTS 1000000   // reconnect times
#define TIMEOUT 30                // 30 seconds timeout
#define MAX_HEARTS 5             // if hearts = 0, means connection losed
#define HEART_BEATS_INTERVAL 10  // how long to send a heart package(seconds)

namespace smfcpp
{
namespace tcp
{

bool recvn(const int sockfd, void * buf, const size_t n, int flags);
bool sendn(const int sockfd, const void * buf, const size_t n, int flags);

enum class PackType {heart, data};
class PackHead
{

public:
    PackHead() = default;
    PackHead(PackType type, uint32_t len);
    virtual 
    ~PackHead() = default;

    PackType get_type();
    uint32_t get_len();

    bool recv_from(const int sockfd);
    bool send_to(const int sockfd);

private:
    PackType m_type;
    uint32_t m_len;
};

class Client : public Node
{
public:
    SMFCPP_SMART_PTR_DEFINITIONS(Client)
    SMFCPP_DISABLE_COPY(Client)
    Client(const std::string & name, 
        const std::string & ip_address, 
        const int port,
        const int timeout = TIMEOUT,
        const int max_reconnects = MAX_RECONNECTS
        );
    virtual ~Client();

    bool connect_to_server();
    bool connect_ensure();
    bool send_data(const std::vector<uint8_t> & data);
    bool recv_data(std::vector<uint8_t> & data);
    virtual void recv_callback(std::vector<uint8_t> & data);
    bool send_heart();
    void keep_alive();

private:
    std::mutex m_mutex; // secure the write and read of socket_fd is thread safe
    
    std::string m_ip_address;
    int m_port;
    struct sockaddr_in m_server_address;

    int m_socket_fd;
    fd_set m_readfds;
    timeval m_timeout;

    std::atomic<int> m_hearts;
    
    bool m_is_connected;
    int m_max_reconnects;

public:
    smfcpp::RecverBase::SharedPtr m_recver;
    smfcpp::TimerBase::SharedPtr m_heart_timer;
    smfcpp::TimerBase::SharedPtr m_keep_alive_timer;
};

class Server : public Node
{
public:
    SMFCPP_SMART_PTR_DEFINITIONS(Server)
    SMFCPP_DISABLE_COPY(Server)
    Server(const std::string & name, int port);
    virtual ~Server()=default;

    void run();
    void losed_clinet_handle(int client_socket_fd);
    bool send_data(int client_socket_fd, const std::vector<uint8_t> & data);
    bool recv_data(int client_socket_fd, std::vector<uint8_t> & data);
    virtual void recv_callback(std::vector<uint8_t> & data);
    void send_heart();
    bool send_heart_once(int client_socket_fd);
    void keep_alive();

private:
    int m_port;
    struct sockaddr_in m_server_address;

    int m_listen_fd;
    int m_max_fd;
    fd_set m_readfds;
    fd_set m_tmpfds;
    timeval m_timeout;

    std::unordered_map<int, std::mutex> m_mutex_mp; // secure the write and read of socket_fd is thread safe
    std::unordered_map<int, std::atomic<int>> m_hearts_mp; // <client_fd, hearts>
    
public:
    smfcpp::RecverBase::SharedPtr m_recver;
    smfcpp::TimerBase::SharedPtr m_heart_timer;
    smfcpp::TimerBase::SharedPtr m_keep_alive_timer;
};

}
}



#endif





