#include <cstdint>
#include <cstring>
#include "pico/time.h"

#include "parsing.h"
#include "constants.h"
#include <files.h>




namespace Parsing{

    volatile ParseState uart_parsing_state = ParseState::WAIT_START;

    volatile uint8_t uart_buffer[uart_buffer_len];
    volatile uint8_t uart_working_index = 0;
    volatile uint8_t payload_len = 0;
    volatile uint32_t command_payload[64]; // len max is 256, so max is /4 of that
};


using namespace Parsing;



extern volatile Animation_Config light_config;
extern volatile uint32_t led_frame[max_frame_len][max_led_len];
extern volatile File files[255];
extern volatile uint32_t data[];
extern volatile uint8_t current_file;
extern volatile uint32_t playback_location;
// extern volatile uint32_t fps_time_ms;


void clear_uart_buffer(){
    // clear out the buffer so old data is not played with again
    for (int i = 0; i < uart_buffer_len; i++){uart_buffer[i]=0;}
    uart_working_index = 0;
    payload_len = 0;

}

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
    switch (uart_parsing_state) {
        case ParseState::WAIT_START:
            if (working_byte == START_CONDITION) {
                uart_working_index = 0;
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

JsonDocument config_set(uint32_t config_id, uint32_t config_value){
    JsonDocument result;
    result["value"] = config_id;
    result["error"] = (uint8_t) ProtoError::OK;
    switch((ConfigIndex) config_id){
        case ConfigIndex::echo:
            // Echoing back the number. useful for debugging
            // result["error"] = (uint8_t) ProtoError::OK;
            // return {config_value, ProtoError::OK};
            break;
        case ConfigIndex::fps_ms:
            if (config_value > 0xFFFF){
                result["error"] = (uint8_t) ProtoError::OUT_OF_RANGE;
                // return {(uint32_t) config_id, ProtoError::OUT_OF_RANGE};
                break;
            }
            if (config_value == 0x00){
                result["error"] = (uint8_t) ProtoError::OUT_OF_RANGE;
                // return {(uint32_t) config_id, ProtoError::OUT_OF_RANGE};
                break;
            }
            light_config.fps_ms = (uint16_t) config_value;
            // fps_time_ms = light_config.fps_ms;
            break;
        case ConfigIndex::running:
            if (config_value > 0x01){
                result["error"] = (uint8_t) ProtoError::OUT_OF_RANGE;
                // return {(uint32_t) config_id, ProtoError::OUT_OF_RANGE};
                break;
            }
            light_config.running = (bool) config_value;
            break;
        case ConfigIndex::led_count:
            light_config.led_count = (uint16_t) config_value;
            break;
        case ConfigIndex::frame_count:
            light_config.frame_count = (uint16_t) config_value;
            break;
        case ConfigIndex::debug_r:
            light_config.debug_r = (uint8_t) config_value;
            break;
        case ConfigIndex::debug_g:
            light_config.debug_g = (uint8_t) config_value;
            break;
        case ConfigIndex::debug_b:
            light_config.debug_b = (uint8_t) config_value;
            break;
        case ConfigIndex::debug_cmd:
            light_config.debug_cmd = (bool) config_value;
            break;
        case ConfigIndex::status_report:
            light_config.status_report = (bool) config_value;
            break;
        case ConfigIndex::current_file:
            current_file = (uint8_t) config_value;
            playback_location = files[current_file].start;
            break;
        default:
            result["error"] = (uint8_t) ProtoError::INVALID_PARAM;
            // return {config_id, ProtoError::INVALID_PARAM};
            break;

    }
    return result;
}

JsonDocument config_get(uint32_t config_id){
    JsonDocument result;
    result["value"] = config_id;
    result["error"] = (uint8_t) ProtoError::OK;
    switch((ConfigIndex)config_id){
        case ConfigIndex::fps_ms:
            result["value"] = light_config.fps_ms;
            // return {(uint32_t) light_config.fps_ms, ProtoError::OK};
            break;
        case ConfigIndex::running:
            result["value"] = light_config.running;
            // return {(uint32_t) light_config.running, ProtoError::OK};
            break;
        case ConfigIndex::led_count:
            result["value"] = light_config.led_count;
            // return {(uint32_t) light_config.led_count, ProtoError::OK};
            break;
        case ConfigIndex::frame_count:
            result["value"] = light_config.frame_count;
            // return {(uint32_t) light_config.frame_count, ProtoError::OK};
            break;
        case ConfigIndex::debug_cmd:
            result["value"] = light_config.debug_cmd;
            // return {(uint32_t) light_config.debug_cmd, ProtoError::OK};
            break;
        case ConfigIndex::status_report:
            result["value"] = light_config.status_report;
            // return {(uint32_t) light_config.debug_cmd, ProtoError::OK};
            break;
        default:
            result["error"] = (uint8_t) ProtoError::INVALID_PARAM;
            // return {(uint32_t) config_id, ProtoError::INVALID_PARAM};
            break;
    }

    return result;

}


JsonDocument color_set(uint32_t frame_id, uint32_t led_id, uint32_t color){
    JsonDocument result;
    result["value"] = led_id;
    result["error"] = (uint8_t) ProtoError::OK;
    if (frame_id > max_frame_len){
        result["value"] = frame_id;
        result["error"] = (uint8_t) ProtoError::OUT_OF_RANGE;
        return result;
        // return {(uint32_t) frame_id, ProtoError::OUT_OF_RANGE};
    }
    if (led_id > max_led_len){
        result["value"] = led_id;
        result["error"] = (uint8_t) ProtoError::OUT_OF_RANGE;
        // return {(uint32_t) led_id, ProtoError::OUT_OF_RANGE};
        return result;
    }
    led_frame[(uint8_t)frame_id][(uint8_t)led_id] = color;
    // return {(uint32_t) led_id, ProtoError::OK};
    return result;
}

JsonDocument multi_color_set(uint32_t frame_id, uint32_t starting_led_id, uint8_t color_array_len, volatile uint32_t* color_array){
    JsonDocument result;
    result["value"] = starting_led_id;
    result["error"] = (uint8_t) ProtoError::OK;
    if (frame_id > max_frame_len){
        result["value"] = frame_id;
        result["error"] = (uint8_t) ProtoError::OUT_OF_RANGE;
        return result;
        // return {(uint32_t) frame_id, ProtoError::OUT_OF_RANGE};
    }
    if (starting_led_id + color_array_len > max_led_len){
        result["value"] = starting_led_id;
        result["error"] = (uint8_t) ProtoError::OUT_OF_RANGE;
        return result;
        // return {(uint32_t) starting_led_id, ProtoError::OUT_OF_RANGE};
    }
    
    uint8_t working_frame_id = (uint8_t) frame_id;
    uint8_t working_led_index= (uint8_t) starting_led_id;

    for (uint8_t i = 0; i<color_array_len ; i++){
        led_frame[working_frame_id][working_led_index+i] = color_array[2+i];
    }
    // return {(uint32_t) starting_led_id, ProtoError::OK};
    return result;
}

JsonDocument file_set(uint32_t file_id, uint32_t starting_location, uint32_t update, uint8_t color_array_len, volatile uint32_t* color_array ){
    JsonDocument result;
    result["value"] = file_id;
    result["error"] = (uint8_t) ProtoError::OK;
    if (file_id > 255){
        result["extra"] = "File Id";
        result["value"] = file_id;
        result["error"] = (uint8_t) ProtoError::OUT_OF_RANGE;
        return result;
        // return {(uint32_t) file_id, ProtoError::OUT_OF_RANGE};
    }
    
    if (starting_location + color_array_len > max_data_len){
        result["extra"] = "Starting Location";
        result["value"] = starting_location;
        result["error"] = (uint8_t) ProtoError::OUT_OF_RANGE;
        return result;
        // return {(uint32_t) starting_location, ProtoError::OUT_OF_RANGE};
    }
    if (update == 1 and files[file_id].end + color_array_len > max_data_len){
        result["extra"] = "Ending Location";
        result["value"] = file_id;
        result["error"] = (uint8_t) ProtoError::OUT_OF_RANGE;
        return result;
        // return {(uint32_t) starting_location, ProtoError::OUT_OF_RANGE};
    }
    uint32_t current_location = 0;
    files[file_id].start = starting_location;
    if (update == 1){
        current_location = files[file_id].end +1;
        files[file_id].end = files[file_id].end + color_array_len  -3;
    }
    else{
        current_location = starting_location;
        files[file_id].end = starting_location + color_array_len - 1 -3;
    }
    
    files[file_id].action = EndAction::REPEAT;
    int temp_index = 0;
    for (int i =3; i<color_array_len; i++ ){
        data[current_location + temp_index] = color_array[i];
        temp_index++;
    }
    // return {file_id, ProtoError::OK};
    return result;
}

JsonDocument color_get(uint32_t frame_id, uint32_t led_id){
    JsonDocument result;
    result["value"] = frame_id;
    result["error"] = (uint8_t) ProtoError::OK;
    if (frame_id > light_config.frame_count){
        result["value"] = frame_id;
        result["error"] = (uint8_t) ProtoError::OUT_OF_RANGE;
        return result;
        // return {(uint32_t) frame_id, ProtoError::OUT_OF_RANGE};
    }
    if (led_id > light_config.led_count){
        result["value"] = led_id;
        result["error"] = (uint8_t) ProtoError::OUT_OF_RANGE;
        return result;
        // return {(uint32_t) led_id, ProtoError::OUT_OF_RANGE};
    }
    // return {led_frame[frame_id][led_id], ProtoError::OK};
    result["value"] = led_frame[frame_id][led_id];
    return result;
}

JsonDocument handle_command(Command& working_command){
    JsonDocument result;
    switch (working_command.id){

        case CommandState::NOOP:
            result["value"] =0;
            result["error"] = (uint8_t) ProtoError::OK;
            break;

        case CommandState::START:
            result["value"] =0;
            result["error"] = (uint8_t) ProtoError::OK;
            break;
        
        case CommandState::CONFIG_SET:
            return config_set(working_command.payload[0], working_command.payload[1]);
        case CommandState::CONFIG_GET:
            return config_get(working_command.payload[0]);
        case CommandState::COLOR_SET:
            return color_set(working_command.payload[0], working_command.payload[1], working_command.payload[2]);
        case CommandState::MULTI_COLOR_SET:
            return multi_color_set(working_command.payload[0], working_command.payload[1], working_command.payload_len, working_command.payload);
        case CommandState::COLOR_GET:
            return color_get(working_command.payload[0], working_command.payload[1]);
        case CommandState::FILE_SET:
            return file_set(working_command.payload[0], working_command.payload[1], working_command.payload[2], working_command.payload_len, working_command.payload);

        default:
            result["value"] = (uint8_t) working_command.id;
            result["error"] = (uint8_t) ProtoError::BAD_COMMAND;
            break;
    }
    return result;

}


JsonDocument parse_payload(volatile uint8_t* data, uint8_t len) {
    JsonDocument result;
    if (data[0] != 0x01){
        result["value"] = 0;
        result["error"] = (uint8_t) ProtoError::BAD_VERSION;
        return result;
    }



    CommandState cmd_id = (CommandState) data[1];

    // TODO: add a for loop to process the bytes into 32 bit ints.
    // send back error if the length isn't a length of 32 bits

    if (len%4 != 0){
        result["value"] = 0;
        result["error"] = (uint8_t) ProtoError::BAD_PAYLOAD_LEN;
        return result;
    }

    for (int i=0; i<64; i++){
        // reset the payload data
        command_payload[i] = 0;
    }

    uint8_t working_array_index = 0;
    uint32_t temp;
    for (int i=0; i<len; i+=4){
        temp = (data[3+i] << 24) | (data[4+i]<<16) | (data[5+i]<<8) | data[6+i];
        command_payload[working_array_index++] = temp;
    }
    
    

    // uint8_t param1 = data[3];
    // uint16_t param2 = (data[4] << 8) | data[5];
    // uint32_t param3 = (data[6] << 24) | (data[7]<<16) | (data[8]<<8) | data[9];

    // switch(len){
    //     // want this to fall through. So no breaks.
    //     case(0x00):
    //         param1 = 0x00;
    //     case(0x01):
    //         param2 = 0x00;
    //     case(0x02):
    //         param3 = 0x00;
    //     default:
    //         break;
    // };
    
    Command working_command = {cmd_id, command_payload, working_array_index};

    result = handle_command(working_command);

    return result;
}
