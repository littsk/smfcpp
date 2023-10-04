#include <iostream>
#include <fstream>
#include <filesystem>

#include <pybind11/pybind11.h>
#include <yaml-cpp/yaml.h>
#include <glog/logging.h>

#include <crc.hpp>
#include <uart.hpp>
#include <sensor.hpp>

#include "config.h"

namespace py = pybind11;

bool change_addr(std::shared_ptr<Uart> uart, uint8_t src, uint8_t dst){
    uint8_t addr_change_frame[8] = {0x01, 0x06, 0x07, 0xd0, 0x00, 0x02, 0x00, 0x00}; //给设备更换地址使用
    uint8_t response_frame[8] = {0};

    addr_change_frame[0] = src;
    addr_change_frame[5] = dst;

    uart->send(addr_change_frame, 8);
    uart->receive(response_frame, 8);

    for(uint32_t i = 0; i < 8; ++i){
        // 如果更换地址成功，应答帧和发送帧相同
        if(addr_change_frame[i] != response_frame[i]){
            return false;
        }
    }

    return true;
}

enum class SensorDeviceType {
    SoilSensor,
    AirSensor
};

// TODO Make it thread safe
bool get_device_cache(YAML::Node& device_cache) {
    std::filesystem::path cache_dir(CACHE_DIR);
    std::filesystem::path device_cache_path = cache_dir / "device_cache.yaml";

    if (!std::filesystem::exists(cache_dir)) {
        LOG(INFO) << "Cache directory didn't exist, creating it now...";
        std::filesystem::create_directories(cache_dir);
        if (std::filesystem::exists(cache_dir)) {
            LOG(INFO) << "Cache directory created successfully";
        } else {
            LOG(ERROR) << "Unknown cache directory creation error";
            return false;
        }
    }

    while (true) {
        try {
            device_cache = YAML::LoadFile(device_cache_path);
            break;
        } catch (YAML::BadFile const& e) {
            LOG(INFO) << "Cache file didn't exist, creating it now...";
            std::ofstream newfile(device_cache_path);
            if (newfile.is_open()) {
                // 文件会在 newfile 作用域结束时自动关闭
                break;
            } else {
                LOG(ERROR) << "Unknown cache file creation error";
                return false;
            }
        } catch (YAML::Exception const& e) {
            LOG(ERROR) << "Failed to load YAML: " << e.what();
            return false;
        }
    }

    return true;
}

bool save_device_cache(YAML::Node& device_cache) {
    std::filesystem::path cache_dir(CACHE_DIR);
    std::filesystem::path device_cache_path = cache_dir / "device_cache.yaml";

    YAML::Emitter emitter;
    emitter << device_cache;

    // 打开文件以进行写入
    std::ofstream fout(device_cache_path);
    if (!fout.is_open()) {
        LOG(ERROR) << "Failed to open the file for writing.";
        return false;
    }

    // 写入数据到文件
    fout << emitter.c_str();

    // 检查写入是否成功
    if (fout.fail()) {
        LOG(ERROR) << "Failed to write data to the file.";
        fout.close();
        return false;
    }

    // 关闭文件
    fout.close();
    return true;
}

bool register_device(SensorDeviceType const type, uint32_t const id) {
    auto uart_sensor_config = smfcpp::UartSensorConfig::get_config_from_device_info();
    if (id > uart_sensor_config.max_addr) {
        LOG(ERROR) << "ID must be less than " << uart_sensor_config.max_addr;
        return false;
    }

    YAML::Node device_cache;
    if (get_device_cache(device_cache) == false) {
        return false;
    }

    std::string device_type = (type == SensorDeviceType::AirSensor) ? "AirSensor" : "SoilSensor";

    if (!device_cache[id]) {
        device_cache[id] = device_type;
        LOG(INFO) << "Registered device at address " << id << ": " << device_type;
    } else {
        std::string old_device_type = device_cache[id].as<std::string>();
        LOG(INFO) << "Registered device failed";
        LOG(INFO) << "Device at address " << id << " is already registered as " << old_device_type;
        LOG(INFO) << "Please remove it first";
        return false;
    }

    if (save_device_cache(device_cache) == false) {
        return false;
    }

    return true;
}

bool reset_device() {
    auto uart_sensor_config = smfcpp::UartSensorConfig::get_config_from_device_info();
    LOG(INFO) << uart_sensor_config.file_path << " "
              << uart_sensor_config.baud_rate << " "
              << uart_sensor_config.n_bits << " "
              << uart_sensor_config.check_method << " "
              << uart_sensor_config.n_stops;

    auto uart = Uart::create_uart(
        uart_sensor_config.file_path,
        uart_sensor_config.baud_rate,
        uart_sensor_config.n_bits,
        uart_sensor_config.check_method,
        uart_sensor_config.n_stops);

    for (int src = 0; src <= uart_sensor_config.max_addr; ++src) {
        if (change_addr(uart, src, 0)) {
            LOG(INFO) << "Reset this device address to 0 SUCCESS";
            return true;
        }
    }

    LOG(ERROR) << "Reset this device address to 0 FAILED";
    return false;
}

bool remove_device(uint32_t const id) {
    auto uart_sensor_config = smfcpp::UartSensorConfig::get_config_from_device_info();
    if (id > uart_sensor_config.max_addr) {
        LOG(ERROR) << "ID must be less than " << uart_sensor_config.max_addr;
        return false;
    }

    YAML::Node device_cache;
    if (get_device_cache(device_cache) == false) {
        return false;
    }

    if (device_cache[id]) {
        std::string device_type = device_cache[id].as<std::string>();
        device_cache.remove(id);
        LOG(INFO) << "Removed device at address " << id << ": " << device_type;
    } else {
        LOG(INFO) << "Device at address " << id << " is not registered.";
        return false;
    }

    if (save_device_cache(device_cache) == false) {
        return false;
    }

    return true;
}

bool list_all_devices() {
    YAML::Node device_cache;
    if (get_device_cache(device_cache) == false) {
        return false;
    }

    LOG(INFO) << "List of all registered devices:";
    for (const auto& entry : device_cache) {
        uint32_t id = entry.first.as<uint32_t>();
        std::string device_type = entry.second.as<std::string>();
        LOG(INFO) << "Device at address " << id << ": " << device_type;
    }

    return true;
}

bool clear_all_devices() {
    YAML::Node device_cache;
    if (get_device_cache(device_cache) == false) {
        return false;
    }

    device_cache.reset();

    if (save_device_cache(device_cache) == false) {
        return false;
    }

    LOG(INFO) << "All registered devices cleared.";

    return true;
}


PYBIND11_MODULE(C_uart, m) {
    py::enum_<SensorDeviceType>(m, "SensorDeviceType")
        .value("SoilSensor", SensorDeviceType::SoilSensor)
        .value("AirSensor", SensorDeviceType::AirSensor)
        .export_values();

    m.def("reset_device", &reset_device, "A function that resets devices");
    m.def("register_device", &register_device, "A function that registers a device");
    m.def("remove_device", &remove_device, "A function that removes a device");
    m.def("list_all_devices", &list_all_devices, "A function that lists all devices");
    m.def("clear_all_devices", &clear_all_devices, "A function that clears all devices");
}