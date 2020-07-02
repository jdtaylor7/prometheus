#ifndef RESOURCE_MANAGER_HPP
#define RESOURCE_MANAGER_HPP

#include <mutex>

class ResourceManager
{
public:
    std::mutex viewer_mode_mutex;
    std::mutex drone_data_mutex;
    std::mutex camera_data_mutex;
};

#endif /* RESOURCE_MANAGER_HPP */
