#include <pybind11/pybind11.h>
#include <yaml-cpp/yaml.h>

#include <iostream>
#include <fstream>
#include <filesystem>

#include <crc.hpp>
#include <uart.hpp>

#include "config.h"

bool change_addr(uint8_t src, uint8_t dst){
    uint8_t addr_change_frame[8] = {0x01, 0x06, 0x07, 0xd0, 0x00, 0x02, 0x00, 0x00}; //给设备更换地址使用
    addr_change_frame[0] = src;
    addr_change_frame[5] = dst;
}

enum class DevType{
    SoilSensor,
    AirSensor
};

// TODO: Make it thread safe
bool get_dev_cache(YAML::Node & dev_cache){
    std::filesystem::path cache_dir(CACHE_DIR);
    std::filesystem::path dev_cache_path = cache_dir / "dev_cache.yaml";

    if (!std::filesystem::exists(cache_dir)) {
        std::cout << "Cache directory didn't exist, creating it now..." << std::endl;
        std::filesystem::create_directories(cache_dir);
    }

    if (std::filesystem::exists(cache_dir)) {
        std::cout << "Cache directory create success" << std::endl;
    } else {
        std::cerr << "Unknown cache directory creation error" << std::endl;
        return false;
    }

    while (true) {
        try {
            dev_cache = YAML::LoadFile(dev_cache_path);
            break;
        } catch (YAML::BadFile const &e) {
            std::cout << "Cache file didn't exist, creating it now..." << std::endl;
            std::ofstream newfile(dev_cache_path);
            if (newfile.is_open()) {
                // 文件会在newfile作用域结束时自动关闭
                break;
            } else {
                std::cerr << "Unknown cache file creation error" << std::endl;
                return false;
            }
        } catch (YAML::Exception const &e) {
            std::cerr << "Failed to load YAML: " << e.what() << std::endl;
            return false;
        }
    }

    return true;
}

bool save_dev_cache(YAML::Node &dev_cache) {
    std::filesystem::path cache_dir(CACHE_DIR);
    std::filesystem::path dev_cache_path = cache_dir / "dev_cache.yaml";

    YAML::Emitter emitter;
    emitter << dev_cache;

    // 打开文件以进行写入
    std::ofstream fout(dev_cache_path);
    if (!fout.is_open()) {
        std::cerr << "Failed to open the file for writing." << std::endl;
        return false;
    }

    // 写入数据到文件
    fout << emitter.c_str();

    // 检查写入是否成功
    if (fout.fail()) {
        std::cerr << "Failed to write data to the file." << std::endl;
        fout.close();
        return false;
    }

    // 关闭文件
    fout.close();
    return true;
}

bool register_dev(DevType const type, uint8_t const id){
    
}

bool reset_dev(){
    // reset dev addr to 0
    std::cout << "hello, world" << std::endl;
}

bool remove_dev(uint8_t id){
    // demove dev from cache

}

namespace py = pybind11;

PYBIND11_MODULE(uart, m) {
    m.doc() = "pybind11 example plugin"; // optional module docstring
    m.def("reset_dev", &reset_dev, "A function that reset_dev");
    m.def("register_dev", &register_dev, "A function that adds two numbers");
}