#ifndef RESOURCE_MANAGER_HPP
#define RESOURCE_MANAGER_HPP

#include <mutex>

class ResourceManager
{
public:
    std::mutex camera_mutex;
};

#endif /* RESOURCE_MANAGER_HPP */
