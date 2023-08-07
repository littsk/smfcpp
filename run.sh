#!/bin/sh

nohup ./smfcpp_server_application/build/uart_server > uart_output.log 2>&1 &
nohup ./smfcpp_server_application/build/camera_server > camera_output.log 2>&1 &