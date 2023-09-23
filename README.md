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

3. Ensure that CMake is installed on your system. Then, compile our project with your desired options. You can choose to compile the smfcpp framework with or without client and server applications by specifying the options during the CMake configuration step:

   - Compile only the smfcpp framework:

     ```shell
     cmake -B build
     ```

   - Compile the smfcpp framework with the client application:

     ```shell
     cmake -B build -DCOMPILE_CLIENT=ON
     ```

   - Compile the smfcpp framework with the server application:

     ```shell
     cmake -B build -DCOMPILE_SERVER=ON
     ```

4. Finally, build the project:

   ```shell
   cmake --build build
   ```

These instructions will help you set up the smfcpp framework on your Linux system with the specified options and build the project accordingly.