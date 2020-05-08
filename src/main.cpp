#include "drone_viewer.hpp"

int main()
{
    DroneViewer viewer{};
    if (!viewer.init()) return -1;

    while (viewer.is_running())
    {
        if (!viewer.process_frame()) return -1;
    }

    return 0;
}
