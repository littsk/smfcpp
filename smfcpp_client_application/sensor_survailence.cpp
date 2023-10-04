#include "sensor.hpp"

#include <executor.hpp>


int main(int argc, char * argv[])
{
    auto uart_sensor_config = smfcpp::UartSensorConfig::get_config_from_device_info();
    auto uart_sensor_client = smfcpp::UartSensorClient::make_shared(
        "uart_sensor_client", uart_sensor_config.server_ip, uart_sensor_config.server_port);
    uart_sensor_client->connect_to_server();
    smfcpp::SingleThreadedExecutor exec;
    exec.add_node(uart_sensor_client);
    exec.spin();
}
