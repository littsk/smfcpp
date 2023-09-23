# smfcpp

### how to install
Now we only suport Linux based system, and we only test on unbuntu 20.04 and 22.04, following shell code is help to install our smfcpp framework

* clone our repo 
```shell
git clone https://github.com/littsk/smfcpp.git
cd smfcpp
```
* install the requirements
```shell
sudo apt install nmap libyaml-cpp-dev libopencv-dev ffmpeg
```
* make sure Cmake installed in your system, then compile our project

```shell
cmake -B build
cmake --build build
```
