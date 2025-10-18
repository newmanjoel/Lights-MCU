#include <cstdint>
#include <cstring>
#include "pico/time.h"

#include "parsing.h"
#include "utils.h"

volatile ParseState uart_parsing_state = ParseState::WAIT_START;
volatile uint8_t uart_buffer[256];
volatile uint8_t uart_working_index = 0;
volatile uint8_t payload_len = 0;
volatile uint32_t last_time;


constexpr char START_CONDITION = 0xAA;
constexpr char END_CONDITION = 0x55;
constexpr uint8_t CRC_LEN = 0x01;
constexpr uint8_t HEADER_LEN = 0x03;

extern volatile Config light_config;

bool verify_crc(volatile uint8_t *input_buffer, uint8_t len){
    // TODO: add some checking here, right now, assume its good for testing
    return true;
}

/*
    [1 B]   [1 B]       [1 B]           [1 B]               [N B]       [2 B]   [1 B]
    [Start] [Version]   [Command ID]    [Payload Length]    [Payload]   [CRC]   [End]
    -       [0]         [1]             [2]                 [3]     

*/

void process_byte(volatile char working_byte){
    last_time = time_us_32();
    switch (uart_parsing_state) {
        case ParseState::WAIT_START:
            if (working_byte == START_CONDITION) {
                uart_working_index = 0;
                // clear out the buffer so old data is not played with again
                // clear_volatile_array(uart_buffer);
                uart_parsing_state = ParseState::READ_HEADER;
            }
            break;

        case ParseState::READ_HEADER:
            uart_buffer[uart_working_index++] = working_byte;
            if (uart_working_index == HEADER_LEN) { // VERSION, CMD_ID, LEN
                payload_len = uart_buffer[2]; // LEN
                if (payload_len == 0x00){
                    uart_parsing_state = ParseState::READ_CRC;
                }
                else{
                    uart_parsing_state = ParseState::READ_PAYLOAD;
                }
            }
            break;

        case ParseState::READ_PAYLOAD:
            uart_buffer[uart_working_index++] = working_byte;
            if (uart_working_index == HEADER_LEN + payload_len) {
                uart_parsing_state = ParseState::READ_CRC;
            }
            break;

        case ParseState::READ_CRC:
            uart_buffer[uart_working_index++] = working_byte;
            if (uart_working_index == HEADER_LEN + CRC_LEN + payload_len) { // CRC low + high
                uart_parsing_state = ParseState::WAIT_END;
            }
            break;

        case ParseState::WAIT_END:
            if (working_byte == END_CONDITION) {
                if (verify_crc(uart_buffer, uart_working_index - 2)) {
                    // handle_message(buffer, idx - 2);
                    uart_parsing_state= ParseState::WAIT_FOR_PROCESSING;
                    
                } else {
                    // send_error_response(ProtoError::BAD_CHECKSUM);
                    // TODO: handle error case
                }
            }
            else{
                uart_parsing_state = ParseState::WAIT_START;
            }
            break;
        case ParseState::WAIT_FOR_PROCESSING:
            // do nothing
            // handled elsewhere
            break;
    }
    
}

Result<uint16_t> config_set(uint8_t config_id, uint16_t config_value){
    switch(config_id){
        case 0x00:
            // Echoing back the number. useful for debugging
            return {config_value, ProtoError::OK};
        case 0x01:
            if (config_value > 0xFF){
                return {(uint16_t) config_id, ProtoError::OUT_OF_RANGE};
            }
            light_config.fps = (uint8_t) config_value;
            break;
        case 0x02:
            if (config_value > 0x01){
                return {(uint16_t) config_id, ProtoError::OUT_OF_RANGE};
            }
            light_config.running = (uint8_t) config_value;
            break;
        case 0x03:
            light_config.led_count = config_value;
            break;
        case 0x04:
            light_config.frame_count = config_value;
            break;
        default:
            return {(uint16_t) config_id, ProtoError::INVALID_PARAM};

    }
    return {config_value, ProtoError::OK};
}

Result<uint16_t> config_get(uint8_t config_id){
    switch((ConfigIndex)config_id){
        case ConfigIndex::fps:
            return {(uint16_t) light_config.fps, ProtoError::OK};
        case ConfigIndex::running:
            return {(uint16_t) light_config.running, ProtoError::OK};
        case ConfigIndex::led_count:
            return {light_config.led_count, ProtoError::OK};
        case ConfigIndex::frame_count:
            return {light_config.frame_count, ProtoError::OK};
        default:
            return {(uint16_t) config_id, ProtoError::INVALID_PARAM};
    }
}

Result<uint16_t> handle_command(Command& working_command){
    switch (working_command.id){

        case CommandState::NOOP:
            return {0, ProtoError::OK};

        case CommandState::START:
            return {0, ProtoError::OK};
        
        case CommandState::CONFIG_SET:
            return config_set(working_command.param1, working_command.param2);
        case CommandState::CONFIG_GET:
            return config_get(working_command.param1);

        
        default:
            return {(uint16_t)working_command.id, ProtoError::BAD_COMMAND};
    }

}


Result<uint16_t> parse_payload(volatile uint8_t* data, uint8_t len) {

    if (data[0] != 0x01){
        return {0, ProtoError::BAD_VERSION};
    }



    CommandState cmd_id = (CommandState) data[1];

    uint8_t param1 = data[3];
    uint16_t param2 = (data[4] << 8) | data[5];
    uint16_t param3 = (data[6] << 8) | data[7];

    switch(len){
        // want this to fall through. So no breaks.
        case(0x00):
            param1 = 0x00;
        case(0x01):
            param2 = 0x00;
        case(0x02):
            param3 = 0x00;
        default:
            break;
    };
    
    Command working_command = {cmd_id, param1, param2, param3,};

    Result<uint16_t> result = handle_command(working_command);

    return result;
}
