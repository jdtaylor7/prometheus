#include "drone_viewer.hpp"

int main()
{
    DroneViewer viewer{};
    if (!viewer.init()) { return -1; }

    while (viewer.is_running())
    {
        viewer.process_frame();
    }

    return 0;
}
