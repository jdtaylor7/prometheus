#ifndef RESOURCE_MANAGER_HPP
#define RESOURCE_MANAGER_HPP

#include <mutex>

class ResourceManager
{
public:
    // std::mutex get_camera_mutex();
public:
    std::mutex camera_mutex;
};

// std::mutex ResourceManager::get_camera_mutex()
// {
//     // std::lock_guard<std::mutex> guard(camera_mutex);
//     // return guard;
//     return camera_mutex;
// }

#endif /* RESOURCE_MANAGER_HPP */
