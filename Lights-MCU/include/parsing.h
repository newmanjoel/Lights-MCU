#ifndef parsing
#define parsing

    #include <cstdint>

    enum class ProtoError : uint8_t{
        OK = 0x00,

        // Protocol-level issues
        BAD_HEADER        = 0x10,
        BAD_CHECKSUM      = 0x11,
        UNEXPECTED_TYPE   = 0x12,
        PAYLOAD_TOO_LONG  = 0x13,
        BAD_VERSION       = 0x14,
        BAD_COMMAND       = 0x15, 

        // Parameter / data issues
        INVALID_PARAM     = 0x20,
        OUT_OF_RANGE      = 0x21,
        MISSING_FIELD     = 0x22,

        // Internal / runtime
        TIMEOUT           = 0x30,
        BUFFER_OVERFLOW   = 0x31,
        UNKNOWN_ERROR     = 0xFF,

    };

    enum class CommandState : uint8_t{
        NOOP = 0x00,
        START = 0x01,
        STOP = 0x02,
        CONFIG_SET = 0x03,
        CONFIG_GET = 0x04,
        COLOR_SET = 0x05,
        COLOR_GET = 0x06,
    };

    enum class ParseState {
        WAIT_START,
        READ_HEADER,
        READ_PAYLOAD,
        READ_CRC,
        WAIT_FOR_PROCESSING,
        WAIT_END
    };

    struct Command {
        CommandState id;
        uint8_t param1;
        uint16_t param2;
        uint16_t param3;
    };

    enum class ConfigIndex : uint8_t{
        fps = 0x01,
        running = 0x02,
        led_count = 0x03,
        frame_count = 0x04,
    };

    struct Config {
        uint8_t fps;
        uint8_t running;
        uint16_t led_count;
        uint16_t frame_count;
    };

    template<typename T>
    struct Result {
        T value;
        ProtoError error;

        bool ok() const { return error == ProtoError::OK; }
    };



    void process_byte(char b);
    Result<uint16_t> parse_payload(volatile uint8_t* data, uint8_t len);

#endif // parsing