#include "log.hpp"

std::string get_current_datetime(){
    time_t now = time(NULL);
    struct tm * tm = localtime(&now);
    char * datetime_str = (char *)malloc(MAX_DATETIME_LEN);
    strftime(datetime_str, MAX_DATETIME_LEN, "%Y-%m-%d %H:%M:%S", tm);
    printf("%s\n", datetime_str);
    std::string res_str(datetime_str);
    free(datetime_str);
    return res_str;
}

log::log(const char * filename, const size_t MaxLogSize, const char *openmode, bool bBackup, bool bEnBuffer){
    if(strlen(filename) < 250){
        memset(this->filename, 0, 301);
        strcpy(this->filename, filename);
    }
    else{
        throw log::error("filename longer than maxsize");
    }

    this->bBackup = bBackup;
    this->bEnBuffer = bEnBuffer;
    this->MaxLogSize = MaxLogSize;

    if((this->tracefp = fopen(filename, openmode)) == NULL){
        perror("fopen");
    }

    memset(this->openmode, 0, 11);
    strcpy(this->openmode, openmode);
}

log::~log(){
    fclose(this->tracefp);
}

bool log::backuplogfile(){
    if(this->bBackup == false || ftell(this->tracefp) < this->MaxLogSize*1024*1024){
        return true;
    } 

    fclose(this->tracefp);

    char strLocalTime[26] = {0};
    get_local_asctime(strLocalTime, 26);
    char bak_filename[301];
    snprintf(bak_filename, 300, "%s.%s", this->filename, strLocalTime);
    rename(this->filename, bak_filename);

    if((this->tracefp = fopen(this->filename, this->openmode)) == NULL){
        perror("fopen");
        return false;
    }

    return true;
}

bool log::write(const char * fmt,...){
    if(this->backuplogfile() == false){
        return false;
    }

    std::string date_str = get_current_datetime();
    va_list ap;
    va_start(ap, fmt);
    fprintf(this->tracefp, "%s\n", date_str.c_str());
    vfprintf(this->tracefp, fmt, ap);
    va_end(ap);

    if (this->bEnBuffer==false) fflush(this->tracefp);

    return true;
}

bool log::write_ex(const char * fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    vfprintf(this->tracefp, fmt, ap);
    va_end(ap);

    if(this->bEnBuffer==false) fflush(this->tracefp);
    return true; 
}



void log::get_local_asctime(char * stime, int slen, const int timetvl){
    if(stime == NULL || slen < 26){
        throw error("stime len must longer than 25");
    }
    time_t timer;
    time(&timer); 
    timer = timer+timetvl;
    strcpy(stime, asctime(localtime(&timer)));
}



