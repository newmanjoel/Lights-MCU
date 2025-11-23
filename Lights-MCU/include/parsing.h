#ifndef parsing
#define parsing

    #include <cstdint>
    #include <hardware/pio.h>
    #include <ArduinoJson-v7.4.2.h>
    #include "constants.h"

    

    enum class ProtoError : uint8_t{
        OK = 0x00,

        // Protocol-level issues
        BAD_HEADER        = 0x10,
        BAD_CHECKSUM      = 0x11,
        UNEXPECTED_TYPE   = 0x12,
        PAYLOAD_TOO_LONG  = 0x13,
        BAD_VERSION       = 0x14,
        BAD_COMMAND       = 0x15, 
        BAD_PAYLOAD_LEN   = 0x16,

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
        MULTI_COLOR_SET = 0x06,
        COLOR_GET = 0x07,
        FILE_SET = 0x08,
        FILE_GET = 0x09,
        FILE_CLEAR = 0x0A,
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
        // uint8_t param1;
        // uint16_t param2;
        // uint32_t param3;
        volatile uint32_t *payload;
        uint8_t payload_len;
    };

    enum class ConfigIndex : uint32_t{
        echo=0x00,
        fps_ms = 0x01,
        running = 0x02,
        led_count = 0x03,
        frame_count = 0x04,
        debug_r = 0x05,
        debug_g = 0x06,
        debug_b = 0x07,
        debug_cmd =0x08,
        status_report = 0x09,
        current_file = 0x0A
    };

    struct Animation_Config {
        uint16_t fps_ms;
        uint16_t led_count;
        uint16_t frame_count;
        uint8_t debug_r;
        uint8_t debug_g;
        uint8_t debug_b;
        bool running;
        bool debug_cmd;
        bool status_report;
        uint8_t current_file;
        
    };

    struct Pio_SM_info{
        PIO pio;
        uint sm;
        uint offset;
    };

    template<typename T>
    struct Result {
        T value;
        ProtoError error;

        bool ok() const { return error == ProtoError::OK; }
    };

    namespace Parsing{

        extern volatile ParseState uart_parsing_state;
    
        extern volatile uint8_t uart_buffer[uart_buffer_len];
        extern volatile uint8_t uart_working_index;
        extern volatile uint8_t payload_len;
        // extern volatile uint32_t time_last_byte_recvd;
        extern volatile uint32_t command_payload[64]; // len max is 256, so max is /4 of that
    };



    void process_byte(char b);
    void parse_payload(JsonDocument& result, volatile uint8_t* data, uint8_t len);

    // template<typename T>
    void clear_uart_buffer(char* buff, uint16_t buff_len);


#endif // parsing