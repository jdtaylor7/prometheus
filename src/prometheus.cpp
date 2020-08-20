#undef NDEBUG

#include "drone_viewer.hpp"
#include "logger.hpp"

Logger logger = Logger(LogLevel::info);

int main()
{
#ifdef TEST_MODE
    logger.log(LogLevel::info, "TEST MODE ACTIVATED!\n");
#endif

    DroneViewer viewer{};
    if (!viewer.init()) return -1;

    while (viewer.is_running())
    {
        if (!viewer.process_frame()) return -1;
    }

    return 0;
}
