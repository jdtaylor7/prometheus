#undef NDEBUG

#include "drone_viewer.hpp"
#include "logger.hpp"

Logger logger = Logger(LogLevel::info);

int main()
{
#ifdef TEST_MODE
    logger.log(LogLevel::info, "Test mode: Enabled\n");
#endif

    DroneViewer viewer{};
    if (!viewer.init()) return -1;

    while (viewer.is_running())
    {
        if (!viewer.process_frame()) return -1;

#ifdef TEST_MODE
        logger.log(LogLevel::info,
            "Test mode: End of first loop. Terminating with no errors\n");
        return 0;
#endif
    }

    return 0;
}
