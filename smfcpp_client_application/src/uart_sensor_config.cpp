#include <filesystem>

#include <yaml-cpp/yaml.h>

#include "sensor.hpp"
#include "config.h"

namespace smfcpp{

UartSensorConfig UartSensorConfig::get_config_from_device_info() {
    std::filesystem::path device_info_path(DEVICE_INFO_PATH);
    YAML::Node device_info = YAML::LoadFile(device_info_path);

    UartSensorConfig uart_sensor_config;
    
    // Load configuration items from the YAML file
    uart_sensor_config.server_ip = device_info["UartSensor"]["ServerIP"].as<std::string>();
    uart_sensor_config.server_port = device_info["UartSensor"]["ServerPort"].as<unsigned short>();
    uart_sensor_config.collect_interval = device_info["UartSensor"]["CollectInterval"].as<std::uint32_t>();
    
    uart_sensor_config.file_path = device_info["UartSensor"]["FilePath"].as<std::string>();
    uart_sensor_config.baud_rate = device_info["UartSensor"]["BaudRate"].as<uint32_t>();
    uart_sensor_config.n_bits = device_info["UartSensor"]["NBits"].as<uint32_t>();
    uart_sensor_config.n_stops = device_info["UartSensor"]["NStops"].as<uint32_t>();
    uart_sensor_config.check_method = device_info["UartSensor"]["CheckMethod"].as<char>();

    uart_sensor_config.max_addr = device_info["UartSensor"]["MaxAddr"].as<uint32_t>();

    return uart_sensor_config;
}

}