#ifndef SMFCPP__CAMERA_CLIENTS_CONSOLE_HPP_
#define SMFCPP__CAMERA_CLIENTS_CONSOLE_HPP_

#include <vector>
#include <sensor.hpp>

namespace smfcpp{

template<CameraType T>
class CameraClientsConsole: public smfcpp::Node
{
public:
    explicit CameraClientsConsole(const std::string & name);

    void run();

    virtual ~CameraClientsConsole();


private:
    std::vector<CameraClient<CameraType::HikVision>::SharedPtr> m_cameras;
};


template<typename T>
class Obj
{
    T a;
};
template class Obj<int>;

#define REGISTER_CameraClientsConsole(T) template class CameraClientsConsole<T>;

}


#endif