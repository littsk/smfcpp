#ifndef SMFCPP__SENSOR_HPP_
#define SMFCPP__SENSOR_HPP_

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>

#include <yaml-cpp/yaml.h>

extern "C" {
    #include <libavutil/time.h>
    #include <libavformat/avformat.h>
    #include <libavutil/timestamp.h> 
    #include <libavformat/avformat.h>    
    #include <libavutil/opt.h>
    #include <libavutil/imgutils.h>
    #include <libswscale/swscale.h>
}

#include <tcp.hpp>
#include <uart.hpp>

namespace smfcpp{

struct UartSensorConfig {
    std::uint32_t farm_id;
    
    std::string server_ip;
    unsigned short server_port;
    // Collect interval time (seconds)
    std::uint32_t collect_interval;

    std::filesystem::path file_path;
    uint32_t baud_rate;
    uint32_t n_bits;
    uint32_t n_stops;
    char check_method;

    uint32_t max_addr;
    
    // Static member function to load configuration from the device information file
    static UartSensorConfig get_config_from_device_info();
};

class UartSensorControl
{
public:
    enum class UartSensorDeviceType {
        SoilSensor,
        AirSensor
    };

    // Device address to UartSensorDeviceType pair
    using id_type_pair = std::pair<uint32_t, std::string>;

    /**
     * Convert a string representation of a sensor device type to the UartSensorDeviceType enum.
     * Throws std::invalid_argument if the input string is not "SoilSensor" or "AirSensor".
     *
     * @param device_type A string representing the sensor device type.
     * @return UartSensorDeviceType enum.
     */
    static UartSensorDeviceType str_to_device_type(std::string const & device_type);

    /**
     * Convert a UartSensorDeviceType enum to a string representation.
     * Throws std::invalid_argument if the input enum is not valid.
     *
     * @param device_type UartSensorDeviceType enum.
     * @return A string representing the sensor device type.
     */
    static std::string device_type_to_str(UartSensorDeviceType const & device_type);

    /**
     * Register a sensor device with the given type and ID.
     *
     * @param type The type of the sensor device (AirSensor or SoilSensor).
     * @param id The unique identifier for the device.
     * @throws std::invalid_argument If the provided ID is greater than the maximum allowed address.
     * @throws std::runtime_error If there is an issue registering the device or saving the device cache.
     */
    static void register_device(UartSensorDeviceType const type, uint32_t const id);

    /**
     * Remove a sensor device with the given ID.
     *
     * @param id The unique identifier of the device to be removed.
     * @throws std::invalid_argument If the provided ID is greater than the maximum allowed address.
     * @throws std::runtime_error If there is an issue removing the device or saving the device cache.
     */
    static void remove_device(uint32_t const id);

    /**
     * Clear all registered sensor devices.
     *
     * @throws std::runtime_error If there is an issue clearing all devices or saving the device cache.
     */
    static void clear_all_devices();

    /**
     * Reset the sensor device to its default state by setting its address to 0.
     * This function is used to reset the device's address for reconfiguration.
     */
    static void reset_device();

    /**
     * List all registered sensor devices along with their addresses and types.
     *
     * @return A vector of pairs, where each pair contains the device's address (ID) and its type (AirSensor or SoilSensor).
     */
    static std::vector<id_type_pair> list_all_devices();

private:
    /**
     * Change the address of a sensor device from the source address to the destination address.
     *
     * @param uart A shared pointer to the UART communication interface.
     * @param src The source address of the device.
     * @param dst The destination address to set for the device.
     * @return Returns true if the address change is successful; otherwise, returns false.
     */
    static bool change_addr(std::shared_ptr<Uart> uart, uint8_t src, uint8_t dst);

    /**
     * Set the address of the sensor device to the specified destination address.
     *
     * @param dst The destination address to set for the device.
     * @throws std::runtime_error If setting the device address fails.
     */
    static void set_addr(uint8_t dst);

    /**
     * Get the device cache from a YAML file.
     *
     * @param device_cache The YAML node to store the retrieved device cache.
     * @throws YAML::Exception If there is an error in loading the device cache from the file.
     */
    static void get_device_cache(YAML::Node& device_cache);

    /**
     * Function to save device cache to a YAML file.
     *
     * @param device_cache The YAML node containing the device cache to be saved.
     * @throws std::runtime_error If there is an error in opening the file for writing or writing data to the file.
     */
    static void save_device_cache(YAML::Node& device_cache);
};

// TODO: 对每一个UART设备名实现线程安全
class UartSensorClient: public tcp::Client
{
public:
    SMFCPP_SMART_PTR_DEFINITIONS(UartSensorClient)
    UartSensorClient(
        const std::string & name, 
        const std::string & ip_address, 
        const int port
    );
    
    virtual ~UartSensorClient() = default;

private:
    /**
     * Collect data from all registered sensor devices and send it to the server.
     * This function retrieves a list of all registered sensor devices, including their addresses and types.
     * It then iterates through the list, collecting data from each device and sending it to the server.
     */
    void collect_data();

    /**
     * Collect air sensor data and send it to the server.
     * This function gathers humidity, temperature, CO2, and light data from the air sensor,
     * constructs a data string, logs the data, and sends it to the server.
     * 
     * @param device_address The address of the sonsor to query
     */
    void collect_air_data(uint8_t device_address);

    /**
     * Collect soil sensor data and send it to the server.
     * This function gathers soil humidity, temperature, electrical conductivity, pH, nitrogen (N), phosphorus (P), and potassium (K) data
     * from the soil sensor, constructs a data string, logs the data, and sends it to the server.
     * 
     * @param device_address The address of the sonsor to query
     */
    void collect_soil_data(uint8_t device_address);

    std::uint32_t m_farm_id;
    std::shared_ptr<Uart> m_uart_port;
    smfcpp::TimerBase::SharedPtr collect_timer;
    smfcpp::RecverBase::SharedPtr device_cache_syncer;
};

enum class CameraType {
    MindVision,
    HikVision
};

struct CameraConfig{
    // these for hikvision
    std::string ip;
    std::string user_name;
    std::string password;
    unsigned short port;

    // these for mindvision
};

template<CameraType type>
class CameraClient{
public:
    SMFCPP_SMART_PTR_DEFINITIONS(CameraClient)

    CameraClient(const std::string & name, 
        const std::string & outputUrl);

    virtual ~CameraClient();

    bool init(const CameraConfig & camera_config);

    int run();

    bool is_open();

    // /**
    //  * \param delay means that if the time exceeds the specified duration, 
    //  *        then the system will give up sending the video.
    // */
    // void send_video(const std::string path, std::chrono::seconds delay);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
    const std::string m_outputUrl;
    bool m_is_open = false;
};

#define REGISTER_CAMERA_CLIENT(T) template class CameraClient<T>;

}

#endif
