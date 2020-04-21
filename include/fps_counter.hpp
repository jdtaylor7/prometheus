#ifndef FPS_COUNTER_HPP
#define FPS_COUNTER_HPP

#include <GLFW/glfw3.h>

class FpsCounter
{
public:
    FpsCounter() : display_fps(false), time(glfwGetTime()) {}

    FpsCounter(bool display_fps_, float update_rate_s_) :
        display_fps(display_fps_),
        update_rate_s(update_rate_s_),
        time(glfwGetTime()) {}

    void update();
private:
    std::size_t frames = 0;
    double time;
    bool display_fps;
    float update_rate_s;
};

void FpsCounter::update()
{
    frames++;
    if ((glfwGetTime() - time) > update_rate_s)
    {
        if (display_fps)
        {
            std::cout << "fps: " << frames / update_rate_s << '\n';
            std::cout << "delta_t: " << glfwGetTime() - time << '\n';
            std::cout << '\n';
        }
        time = glfwGetTime();
        frames = 0;
    }
}

#endif /* FPS_COUNTER_HPP */
