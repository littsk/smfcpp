#include "sensor_server.hpp"

#include <log.hpp>

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>

using namespace smfcpp;

void get_files(const std::string dirname, std::vector<std::string> &filelist)
{
    if(dirname.empty())
        return;
    struct stat s;
    stat(dirname.c_str(), &s);
    if(!S_ISDIR(s.st_mode))
        return;
    DIR *dirhand = opendir(dirname.c_str());
    if(NULL == dirhand){
        exit(EXIT_FAILURE);
    }
    dirent *fp = nullptr;
    while((fp = readdir(dirhand)) != nullptr){
        if(fp->d_name[0] != '.'){//十分重要的一行（？）
            std::string filename = dirname + "/" + std::string(fp->d_name);
            struct stat filemod;
            stat(filename.c_str(), &filemod);
            if(S_ISDIR(filemod.st_mode)){
                get_files(filename, filelist);
            }
            else if(S_ISREG(filemod.st_mode)){
                filelist.push_back(filename);
            }
        }
    }
    closedir(dirhand);
    return;
}

UartServer::UartServer(
    const std::string & name, 
    int port,
    const std::string & file_name)
: tcp::Server(name, port),
  logfile(new log(file_name.c_str()))
{}

UartServer::~UartServer()
{
    delete logfile;
}

void UartServer::recv_callback(
    std::vector<uint8_t> & data)
{
    std::string tmp(data.begin(), data.end());
    logfile->write("%s\n", tmp.c_str());
}

CameraServer::CameraServer(
    const std::string & name, 
    int port,
    size_t max_files)
: tcp::Server(name, port),
  m_max_files(max_files)
{}

void CameraServer::recv_callback(
    std::vector<uint8_t> & data)
{
    std::string date_str = get_current_datetime();
    std::string pth = "./video/video_surveillance_" + date_str + ".mp4";
    printf("%s\n", pth.c_str());

    int fd;
    if((fd = open(pth.c_str(), O_RDWR | O_CREAT)) < 0){
        perror("open");
    }
    size_t wr_size = write(fd, data.data(), data.size());
    close(fd);
    
    std::vector<std::string> file_list;
    get_files("./video", file_list);
    if(file_list.size() > m_max_files){
        sort(file_list.begin(), file_list.end());
        for(int i = 0; i < m_max_files / 2; ++i){
            remove(file_list[i].c_str());
        }
    }
}

