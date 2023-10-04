#include <sstream>
#include <iomanip>
#include <filesystem>

#include <sys/inotify.h>

#include <glog/logging.h>

#include <crc.hpp>

#include "sensor.hpp"
#include "config.h"

static uint8_t air_acquire[8] = {0x02, 0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00};
static uint8_t soil_acquire[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00};
CRC::Mod<uint16_t> mod(0x8005, 0xffff, 0x0000, true, true);

namespace smfcpp{

UartSensorClient::UartSensorClient(
    const std::string & name, 
    const std::string & ip_address, 
    const int port)
: tcp::Client(name, ip_address, port)
{   
    auto uart_sensor_config = smfcpp::UartSensorConfig::get_config_from_device_info();

    // Create a UART communication interface based on the configuration
    auto m_uart_port = Uart::create_uart(
        uart_sensor_config.file_path,
        uart_sensor_config.baud_rate,
        uart_sensor_config.n_bits,
        uart_sensor_config.check_method,
        uart_sensor_config.n_stops);

    collect_timer = this->create_timer(
        std::chrono::seconds(uart_sensor_config.collect_interval), std::bind(&UartSensorClient::collect_data, this));
    
    /**
     * Lambda function for monitoring whether a file has changed.
     *
     * @param filename The path and name of the file to monitor.
     * @return Returns true when the file changes; the function blocks the current process until a change occurs.
     * @throws std::runtime_error If an error occurs during inotify initialization, adding an inotify watch, or reading inotify events,
     *         an exception is thrown with an error description.
     */
    auto device_cache_modified_monitor = []() -> bool {

        std::filesystem::path device_cache_path(DEVICE_CACHE_PATH);

        int inotifyFd = inotify_init();
        if (inotifyFd == -1) {
            throw std::runtime_error("Error initializing inotify: " + std::string(strerror(errno)));
        }

        int watchFd = inotify_add_watch(inotifyFd, device_cache_path.string().c_str(), IN_MODIFY);
        if (watchFd == -1) {
            throw std::runtime_error("Error adding inotify watch: " + std::string(strerror(errno)));
        }

        char buffer[sizeof(struct inotify_event) + NAME_MAX + 1];
        ssize_t bytesRead = read(inotifyFd, buffer, sizeof(buffer));
        if (bytesRead == -1) {
            throw std::runtime_error("Error reading inotify events: " + std::string(strerror(errno)));
        }

        close(inotifyFd);
        return true;
    };

    /**
     * Synchronize the device cache to server when the device cache changed.
     */
    auto sync_device_cache = [this]() -> void {
        // Get a list of all registered sensor devices with their addresses and types
        auto all_devices = UartSensorControl::list_all_devices();

        // Construct a string representation of all devices
        std::stringstream all_devices_ss;
        for (auto const & device : all_devices) {
            all_devices_ss << "Device at address " << device.first << ": " << device.second << "\n";
        }
        auto all_devices_str = all_devices_ss.str();

        // Log the updated device cache
        LOG(INFO) << "Updated device cache:\n" << all_devices_str;

        // Convert the device cache string into a vector of bytes and send it
        std::vector<uint8_t> all_devices_vec(all_devices_str.begin(), all_devices_str.end());
        send_data(all_devices_vec);
    };

    device_cache_syncer = this->create_recver(device_cache_modified_monitor, sync_device_cache);
}

void UartSensorClient::collect_data() {
    // Get a list of all registered sensor devices with their addresses and types
    auto all_devices = UartSensorControl::list_all_devices();

    using DeviceType = UartSensorControl::UartSensorDeviceType;

    for (auto const & device : all_devices) {
        uint32_t device_address = device.first;
        auto device_type = UartSensorControl::str_to_device_type(device.second);

        if (device_type == DeviceType::AirSensor) {
            // Collect data from an air sensor device
            collect_air_data(device_address);
        } else {
            // Collect data from a soil sensor device
            collect_soil_data(device_address);
        }
    }
}

void UartSensorClient::collect_air_data(uint8_t device_address) {
    std::vector<uint8_t> m_sensor_data(11, 0);

    // Set address and perform crc checking
    air_acquire[0] = device_address;
    CRC::crc_comlete(air_acquire, 6, mod);

    // Collect air sensor data
    m_uart_port->send(air_acquire, 8);
    m_uart_port->receive(m_sensor_data.data(), 13);

    // Extract data values
    float hum = (int16_t)((m_sensor_data[3] << 8) | m_sensor_data[4]) / 10.0; 
    float tem = (int16_t)((m_sensor_data[5] << 8) | (m_sensor_data[6])) / 10.0;
    float co2 = (uint16_t)((m_sensor_data[7] << 8) | (m_sensor_data[8]));
    float light = (uint16_t)((m_sensor_data[9] << 8) | (m_sensor_data[10]));

    // Create a data string
    std::stringstream air_ss;
    air_ss << "humidity:" << std::fixed << std::setprecision(2) << hum << "%,"
        << "temperature:" << std::fixed << std::setprecision(2) << tem << "centigrade,"
        << "co2:" << std::fixed << std::setprecision(2) << co2 << "ppm,"
        << "light:" << std::fixed << std::setprecision(2) << light << "lux";

    std::string air_str = air_ss.str();
    std::vector<uint8_t> air_data_vec(air_str.begin(), air_str.end());

    // Log the data
    LOG(INFO) << "Air Sensor Data:\n" << air_str;

    // Send data to the server
    send_data(air_data_vec);
}

void UartSensorClient::collect_soil_data(uint8_t device_address) {
    std::vector<uint8_t> m_sensor_data(17, 0);

    // Set address and perform crc checking
    soil_acquire[0] = device_address;
    CRC::crc_comlete(soil_acquire, 6, mod);

    // Collect soil sensor data
    m_uart_port->send(soil_acquire, 8);
    m_uart_port->receive(m_sensor_data.data(), 19);

    // Extract data values
    float soil_hum = (int16_t)((m_sensor_data[3] << 8) | m_sensor_data[4]) / 10.0; 
    float soil_tem = (int16_t)((m_sensor_data[5] << 8) | (m_sensor_data[6])) / 10.0;
    float soil_ec = (uint16_t)((m_sensor_data[7] << 8) | (m_sensor_data[8]));
    float soil_ph = (uint16_t)((m_sensor_data[9] << 8) | (m_sensor_data[10])) / 10.0;
    float soil_N = (uint16_t)((m_sensor_data[11] << 8) | (m_sensor_data[12]));
    float soil_P = (uint16_t)((m_sensor_data[13] << 8) | (m_sensor_data[14]));
    float soil_K = (uint16_t)((m_sensor_data[15] << 8) | (m_sensor_data[16]));

    // Create a data string
    std::stringstream soil_ss;
    soil_ss << "soil humidity:" << std::fixed << std::setprecision(2) << soil_hum << "%,"
        << "soil temperature:" << std::fixed << std::setprecision(2) << soil_tem << "centigrade,"
        << "soil electrical conductivity:" << std::fixed << std::setprecision(2) << soil_ec << "us/cm,"
        << "PH:" << std::fixed << std::setprecision(2) << soil_ph << ","
        << "N:" << std::fixed << std::setprecision(2) << soil_N << "mg/kg,"
        << "P:" << std::fixed << std::setprecision(2) << soil_P << "mg/kg,"
        << "K:" << std::fixed << std::setprecision(2) << soil_K << "mg/kg";

    std::string soil_str = soil_ss.str();

    // Log the data
    LOG(INFO) << "Soil Sensor Data:\n" << soil_str;

    std::vector<uint8_t> soil_data_vec(soil_str.begin(), soil_str.end());

    // Send data to the server
    send_data(soil_data_vec);
}

}