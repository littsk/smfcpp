#include <iostream>
#include <utility>
#include <fstream>
#include <filesystem>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <yaml-cpp/yaml.h>
#include <glog/logging.h>

#include <crc.hpp>
#include <uart.hpp>
#include <sensor.hpp>

#include "config.h"

namespace py = pybind11;

using UartSensorDeviceType = smfcpp::UartSensorControl::UartSensorDeviceType;

PYBIND11_MODULE(C_uart, m) {
    py::enum_<UartSensorDeviceType>(m, "UartSensorDeviceType")
        .value("SoilSensor", UartSensorDeviceType::SoilSensor)
        .value("AirSensor", UartSensorDeviceType::AirSensor)
        .export_values();

    m.def("reset_device", &smfcpp::UartSensorControl::reset_device, "A function that resets devices");
    m.def("register_device", &smfcpp::UartSensorControl::register_device, "A function that registers a device");
    m.def("remove_device", &smfcpp::UartSensorControl::remove_device, "A function that removes a device");
    m.def("list_all_devices", &smfcpp::UartSensorControl::list_all_devices, "A function that lists all devices");
    m.def("clear_all_devices", &smfcpp::UartSensorControl::clear_all_devices, "A function that clears all devices");
}