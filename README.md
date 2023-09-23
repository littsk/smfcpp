# smfcpp

### Installation Instructions
Currently, our smfcpp framework is supported only on Linux-based systems. We have tested it on Ubuntu 20.04 and 22.04. The following shell commands will guide you through the installation process:

1. Clone our repository:

   ```shell
   git clone https://github.com/littsk/smfcpp.git
   cd smfcpp
   ```

2. Install the required dependencies:

   ```shell
   sudo apt install nmap libyaml-cpp-dev libopencv-dev ffmpeg
   ```

3. Ensure that CMake is installed on your system. Then, compile our project:

   ```shell
   cmake -B build
   ```

   You can choose to compile the smfcpp framework with or without client and server applications. Use one of the following commands based on your requirements:

   - Compile only the smfcpp framework:

     ```shell
     cmake --build build
     ```

   - Compile the smfcpp framework with the client application:

     ```shell
     cmake --build build -DCOMPILE_CLIENT=NO
     ```

   - Compile the smfcpp framework with the server application:

     ```shell
     cmake --build build -DCOMPILE_SERVER=NO
     ```

These instructions will help you set up the smfcpp framework on your Linux system.