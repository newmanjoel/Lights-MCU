#ifndef COMMAND_STRUCT
#define COMMAND_STRUCT

#include <cstdint>

struct Command {
    uint8_t id;
    uint8_t param1;
    uint16_t param2;
    uint16_t param3;
};

struct Config {
    uint8_t fps;
    uint16_t led_count;
    uint16_t frame_count;
};

#endif // COMMAND_STRUCT
