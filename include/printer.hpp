#ifndef PRINTER_HPP
#define PRINTER_HPP

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

#include "camera.hpp"
#include "resource_manager.hpp"

class Printer
{
public:
    Printer(ResourceManager& rm_, Camera& camera_) : camera(camera_), rm(rm_) {}

    void print_camera_details();
private:
    ResourceManager& rm;
    Camera& camera;

    std::atomic_flag camera_print_recent;
};

void Printer::print_camera_details()
{
    if (!camera_print_recent.test_and_set())
    {
        std::lock_guard<std::mutex> g(rm.camera_mutex);

        glm::vec3 pos = camera.get_pos();
        glm::vec3 target = camera.get_pos() + camera.get_front();

        std::cout << "pos.x = " << pos.x << '\n';
        std::cout << "pos.y = " << pos.y << '\n';
        std::cout << "pos.z = " << pos.z << '\n';

        std::cout << "target.x = " << target.x << '\n';
        std::cout << "target.y = " << target.y << '\n';
        std::cout << "target.z = " << target.z << '\n';
        std::cout << '\n';

        std::thread t1([this](){
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            camera_print_recent.clear();
        });
        t1.detach();
    }
}

#endif /* PRINTER_HPP */
