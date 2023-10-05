#include <glog/logging.h>

#include <crc.hpp>

#include "sensor.hpp"
#include "config.h"


static CRC::Mod<uint16_t> mod(0x8005, 0xffff, 0x0000, true, true);

namespace smfcpp
{

UartSensorControl::UartSensorDeviceType UartSensorControl::str_to_device_type(std::string const & device_type) {
    if (device_type == "SoilSensor") {
        return UartSensorDeviceType::SoilSensor;
    } else if (device_type == "AirSensor") {
        return UartSensorDeviceType::AirSensor;
    } else {
        throw std::invalid_argument("Invalid sensor device type: " + device_type);
    }
}

std::string UartSensorControl::device_type_to_str(UartSensorDeviceType const & device_type) {
    switch (device_type) {
        case UartSensorDeviceType::SoilSensor:
            return "SoilSensor";
        case UartSensorDeviceType::AirSensor:
            return "AirSensor";
        default:
            throw std::invalid_argument("Invalid UartSensorDeviceType enum.");
    }
}

bool UartSensorControl::change_addr(std::shared_ptr<Uart> uart, uint8_t src, uint8_t dst) {
    // Frame for changing device address
    uint8_t addr_change_frame[8] = {0x01, 0x06, 0x07, 0xd0, 0x00, 0x02, 0x00, 0x00};
    addr_change_frame[0] = src;
    addr_change_frame[5] = dst;
    CRC::crc_comlete(addr_change_frame, 6, mod);

    uint8_t response_frame[8] = {0};

    uart->send(addr_change_frame, 8);
    uart->receive(response_frame, 8);

    // Check if the response matches the sent frame to verify address change success
    for (uint32_t i = 0; i < 8; ++i) {
        if (addr_change_frame[i] != response_frame[i]) {
            return false;
        }
    }

    return true;
}

void UartSensorControl::set_addr(uint8_t dst) {
    auto uart_sensor_config = smfcpp::UartSensorConfig::get_config_from_device_info();
    LOG(INFO) << uart_sensor_config.file_path << " "
              << uart_sensor_config.baud_rate << " "
              << uart_sensor_config.n_bits << " "
              << uart_sensor_config.check_method << " "
              << uart_sensor_config.n_stops;

    // Create a UART communication interface based on the configuration
    auto uart = Uart::create_uart(
        uart_sensor_config.file_path,
        uart_sensor_config.baud_rate,
        uart_sensor_config.n_bits,
        uart_sensor_config.check_method,
        uart_sensor_config.n_stops);

    // Attempt to change the address for all possible source addresses
    for (int src = 0; src <= uart_sensor_config.max_addr; ++src) {
        if (change_addr(uart, src, dst)) {
            LOG(INFO) << "Set this device address to " << uint32_t(dst) << " SUCCESS";
            return;
        }
    }

    // Address change failed for all source addresses
    LOG(ERROR) << "Set this device address to " << uint32_t(dst) << " FAILED";
    throw std::runtime_error("Failed to set the device address to " + std::to_string(uint32_t(dst)));
}

void UartSensorControl::get_device_cache(YAML::Node& device_cache) {
    std::filesystem::path cache_dir(CACHE_DIR);
    std::filesystem::path device_cache_path(DEVICE_CACHE_PATH);

    // Create the cache directory if it doesn't exist
    if (!std::filesystem::exists(cache_dir)) {
        LOG(INFO) << "Cache directory didn't exist, creating it now...";
        std::filesystem::create_directories(cache_dir);
        LOG(INFO) << "Cache directory created successfully";
    }

    try {
        // Load the device cache from the YAML file
        device_cache = YAML::LoadFile(device_cache_path);
        return;
    } catch (YAML::BadFile const& e) {
        // If the cache file doesn't exist, create it and then load it
        LOG(INFO) << "Cache file didn't exist, creating it now...";
        std::ofstream newfile(device_cache_path);
        LOG(INFO) << "Cache file created successfully";
        device_cache = YAML::LoadFile(device_cache_path);
    } catch (YAML::Exception const& e) {
        throw e;
    }
}

void UartSensorControl::save_device_cache(YAML::Node& device_cache) {
    std::filesystem::path device_cache_path(DEVICE_CACHE_PATH);

    YAML::Emitter emitter;
    emitter << device_cache;

    // Open the file for writing
    std::ofstream fout(device_cache_path);
    if (!fout.is_open()) {
        throw std::runtime_error("Failed to open the file for writing.");
    }

    // Write data to the file
    fout << emitter.c_str();

    // Check if writing was successful
    if (fout.fail()) {
        fout.close();
        throw std::runtime_error("Failed to write data to the file.");
    }

    // Close the file
    fout.close();
}

void UartSensorControl::register_device(UartSensorDeviceType const type, uint32_t const id) {
    // Get UART sensor configuration
    auto uart_sensor_config = smfcpp::UartSensorConfig::get_config_from_device_info();

    // Check if the provided ID is within the valid range
    if (id > uart_sensor_config.max_addr) {
        throw std::invalid_argument("ID must be less than " + std::to_string(uart_sensor_config.max_addr));
    }

    // Load the existing device cache
    YAML::Node device_cache;
    get_device_cache(device_cache);

    // change UartSensorDeviceType to string for save to yaml
    std::string device_type = device_type_to_str(type);

    // Register the device if it is not already registered
    if (!device_cache[id]) {
        device_cache[id] = device_type;
        set_addr(id);
        LOG(INFO) << "Registered device at address " << id << ": " << device_type;
    } else {
        // Device with the given ID is already registered
        std::string old_device_type = device_cache[id].as<std::string>();
        LOG(ERROR) << "Registered device failed" << "Device at address " 
                   << id << " is already registered as " << old_device_type
                   << " Please remove it first.";
        throw std::runtime_error("Device at address " + std::to_string(id) + " is already registered.");
    }

    // Save the updated device cache
    save_device_cache(device_cache);
}

void UartSensorControl::remove_device(uint32_t const id) {
    // Get UART sensor configuration
    auto uart_sensor_config = smfcpp::UartSensorConfig::get_config_from_device_info();

    // Check if the provided ID is within the valid range
    if (id > uart_sensor_config.max_addr) {
        throw std::invalid_argument("ID must be less than " + std::to_string(uart_sensor_config.max_addr));
    }

    // Load the existing device cache
    YAML::Node device_cache;
    get_device_cache(device_cache);

    // Remove the device if it is registered
    if (device_cache[id]) {
        std::string device_type = device_cache[id].as<std::string>();
        device_cache.remove(id);
        LOG(INFO) << "Removed device at address " << id << ": " << device_type;
    } else {
        // Device with the given ID is not registered
        LOG(ERROR) << "Device at address " << id << " is not registered.";
        throw std::runtime_error("Device at address " + std::to_string(id) + " is not registered.");
    }

    // Save the updated device cache
    save_device_cache(device_cache);
}

void UartSensorControl::clear_all_devices() {
    // Load the existing device cache
    YAML::Node device_cache;
    get_device_cache(device_cache);

    // Clear all registered devices
    device_cache.reset();

    // Save the updated device cache
    save_device_cache(device_cache);

    LOG(INFO) << "All registered devices cleared.";
}

void UartSensorControl::reset_device() {
    set_addr(0);
}

std::vector<UartSensorControl::id_type_pair> UartSensorControl::list_all_devices() {
    YAML::Node device_cache;
    get_device_cache(device_cache);
    std::vector<id_type_pair> result;

    LOG(INFO) << "List of all registered devices:";

    for (const auto & entry : device_cache) {
        uint32_t id = entry.first.as<uint32_t>();
        std::string device_type = entry.second.as<std::string>();
        result.emplace_back(id, device_type);
        LOG(INFO) << "Device at address " << id << ": " << device_type;
    }

    return result;
}

}

