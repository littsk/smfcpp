#ifndef LOG_HPP
#define LOG_HPP

#define __STDC_LIB_EXT1__ 1
#define __STDC_WANT_LIB_EXT1__ 1

#include <iostream>
#include <stdexcept>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>


#define MAX_DATETIME_LEN 20 // yyyy-mm-dd hh:mm:ss 加上'\0'的长度

std::string get_current_datetime();

class log{
public:
    class error : public std::logic_error{
    public:
        error(const std::string & str):logic_error(str){}
        virtual ~error() noexcept {}

    };

    log(const char * filename, const size_t MaxLogSize = 100, 
        const char * openmode = "a+", bool bBackup = true, bool bEnBuffer = false);
    ~log();
    bool write(const char *fmt,...);
    bool write_ex(const char *fmt,...);

    void get_local_asctime(char * stime, int slen, const int timetvl = 0);

private:
    FILE *  tracefp;           
    char    filename[301]; 
    char    openmode[11];    
    bool    bEnBuffer;   // whether to use os buffer machanism    
    long    MaxLogSize;        
    bool    bBackup; 

    bool backuplogfile();
};

#endif