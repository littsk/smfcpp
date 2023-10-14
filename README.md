# smfcpp

### Installation Instructions
Currently, our smfcpp framework is supported only on Linux-based systems. We have tested it on Ubuntu 20.04 and 22.04. The following shell commands will guide you through the installation process:

1. Clone our repository:

   ```shell
   git clone https://github.com/littsk/smfcpp.git
   git submodule update --init
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
5. Optionally, if you wish to use this project with the Python extension, you can install the required Python runtime scripts (only applicable to the client):
   ```shell
   python3 pip install -r requirements.txt
   python3 pip instll .
   ```
   Then, you can run the provided scripts directly from the command line:
   ```shell
   my_script
   ```
These instructions will guide you through the setup of the smfcpp framework on your Linux system, allowing you to choose specific options during the installation process.
