#include <node.hpp>

#include <memory>
#include <iostream>
#include <typeinfo>
#include <string>

#include <boost/core/demangle.hpp>

#include <unistd.h>

template<typename T>
void print_type(const T & x){
    const std::string real_name = boost::core::demangle(typeid(x).name());
    std::cout<<real_name<<std::endl;
}



int main(int agrc, char * argv[]){
}