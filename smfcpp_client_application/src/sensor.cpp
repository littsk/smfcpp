#include <sstream>
#include <iomanip>

#include <sys/stat.h>

#include <yaml-cpp/yaml.h>

#include <CameraApi.h>
#include <crc.hpp>

#include "sensor.hpp"
#include "config.h"

static uint8_t air_acquire[8] = {0x02, 0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00};
static uint8_t soil_acquire[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00};
static uint8_t addr1to2[8]    = {0x01, 0x06, 0x07, 0xd0, 0x00, 0x02, 0x00, 0x00}; //给设备更换地址使用
CRC::Mod<uint16_t> mod(0x8005, 0xffff, 0x0000, true, true);

namespace smfcpp{

UartClient::UartClient(
    const std::string & name, 
    const std::string & ip_address, 
    const int port,
    const std::string & uart_file,
    const int collect_interval)
: tcp::Client(name, ip_address, port)
{
    m_uart_port = new Uart(uart_file.c_str(), 4800, 8, 'N', 1);
    // CRC::crc_comlete(addr1to2, 6, mod);
    // m_uart_port->send(addr1to2, 8);
    collect_timer = this->create_timer(
        std::chrono::seconds(collect_interval), std::bind(&UartClient::collect_data, this));
}

UartClient::~UartClient(){
    delete m_uart_port;
}

void UartClient::collect_data(){
    std::vector<uint8_t> m_sensor_data(24, 0);

    CRC::crc_comlete(air_acquire, 6, mod);
    CRC::crc_comlete(soil_acquire, 6, mod);

    // air data
    m_uart_port->send(air_acquire, 8);
    m_uart_port->receive(m_sensor_data.data(), 13);
    float hum = (int16_t)((m_sensor_data[3] << 8) | m_sensor_data[4]) / 10.0, 
          tem = (int16_t)((m_sensor_data[5] << 8) | (m_sensor_data[6])) / 10.0,
          co2 = (uint16_t)((m_sensor_data[7] << 8) | (m_sensor_data[8])),
          light = (uint16_t)((m_sensor_data[9] << 8) | (m_sensor_data[10])); // | (m_sensor_data[11] << 8) | (m_sensor_data[12]));
    std::stringstream air_ss;
    air_ss << "humidity:" << std::fixed << std::setprecision(2) << hum << "%,"
        << "temperature:" << std::fixed << std::setprecision(2) << tem << "centigrade,"
        << "co2:" << std::fixed << std::setprecision(2) << co2 << "ppm,"
        << "light:" << std::fixed << std::setprecision(2) << light << "lux";
    std::string air_str = air_ss.str();
    std::vector<uint8_t> air_data_vec(air_str.begin(), air_str.end());
    std::cout << air_str << std::endl;
    send_data(air_data_vec);     // send_data_to_server

    // soil_data
    m_uart_port->send(soil_acquire, 8);
    m_uart_port->receive(m_sensor_data.data(), 19);
    float soil_hum = (int16_t)((m_sensor_data[3] << 8) | m_sensor_data[4]) / 10.0, 
          soil_tem = (int16_t)((m_sensor_data[5] << 8) | (m_sensor_data[6])) / 10.0,
          soil_ec = (uint16_t)((m_sensor_data[7] << 8) | (m_sensor_data[8])),
          soil_ph = (uint16_t)((m_sensor_data[9] << 8) | (m_sensor_data[10])) / 10.0,
          soil_N = (uint16_t)((m_sensor_data[11] << 8) | (m_sensor_data[12])),
          soil_P = (uint16_t)((m_sensor_data[13] << 8) | (m_sensor_data[14])),
          soil_K = (uint16_t)((m_sensor_data[15] << 8) | (m_sensor_data[16]));
    std::stringstream soil_ss;
    soil_ss << "soil humidity:" << std::fixed << std::setprecision(2) << soil_hum << "%,"
        << "soil temperature:" << std::fixed << std::setprecision(2) << soil_tem << "centigrade,"
        << "soil electrical conductivity:" << std::fixed << std::setprecision(2) << soil_ec << "us/cm,"
        << "PH:" << std::fixed << std::setprecision(2) << soil_ph << ","
        << "N:" << std::fixed << std::setprecision(2) << soil_N << "mg/kg,"
        << "P:" << std::fixed << std::setprecision(2) << soil_P << "mg/kg,"
        << "K:" << std::fixed << std::setprecision(2) << soil_K << "mg/kg";
    std::string soil_str = soil_ss.str();
    std::cout << soil_str << std::endl;
    std::vector<uint8_t> soil_data_vec(soil_str.begin(), soil_str.end());
    send_data(soil_data_vec); // send_data_to_server
}

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