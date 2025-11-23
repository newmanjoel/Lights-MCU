from enum import Enum
import json
import serial
import struct
import time
import re
import numpy as np
from itertools import islice
from structs import Commands, ConfigIndex, ProtoError

import logging

logger = logging.getLogger("Light-MCU-Testing")


# Define struct format:
# <  = little-endian
# B  = uint8_t
# H  = uint16_t
# Structure: id, param1, param2, param3
PACKET_FORMAT = ">5B2HBB"
start_packet = 0xAA
end_packet = 0x55
version = 0x01

serial_delimiter = b'\n'

type CommandValue = int|Enum


def send_command(ser:serial.Serial, id:CommandValue, data:list[CommandValue|int]):
    """Send a Command struct over Serial in binary format."""
    if isinstance(id, Enum):
        id = id.value
    assert(type(id) is int)

    # assert(type(param3) is int)
    int_data = []

    for thing in data:
        if isinstance(thing, Enum):
            int_data.append(thing.value)
        elif type(thing) == int:
            int_data.append(thing)
        else:
            raise ValueError(f"Unknown data type of {type(thing)}")

    # sending 32 bit ints, so the length is in bytes
    data_len = len(data)

    packet_format = f'>4B{data_len}I2B'
    packet = struct.pack(packet_format,
                         start_packet,
                         version,
                         id,
                         data_len*4, 
                         *int_data,
                         0xFF,
                         end_packet)
    

    # packet_format = PACKET_FORMAT
    # if param2 == -1 and param3 == -1:
    #     packet_format = ">5BBB"
    #     logger.debug(f"Packet format: {packet_format}")
    #     packet = struct.pack(packet_format, 
    #         start_packet, 
    #         version,
    #         id, 
    #         0x01, # length in bytes
    #         param1,
    #         0xFF, # CRC
    #         end_packet
    #     )
    # else:
    #     packet_format = ">5BHIBB"
    #     packet = struct.pack(packet_format, 
    #         start_packet, 
    #         version,
    #         id, 
    #         0x07, # length in bytes
    #         param1, #1 B
    #         param2, #2 B
    #         param3, #4 B
    #         0xFF, # CRC
    #         end_packet
    #     )
    
    result = ser.write(packet)
    ser.flush()  # ensure all bytes are sent
    logger.getChild("->").debug(f"{result} '{packet.hex(":")=}'")
    # time.sleep(0.2)

def read_command(ser:serial.Serial,timeout=1.5):
    ser.timeout = timeout
    data = ser.read_until(serial_delimiter)
    str_data = data.decode().strip()
    if str_data.startswith("Value:"):
        split_str = re.findall(r"Value: (?P<value>\w+), Error: (?P<error>\w+)", str_data)
        # split_str=[('002A', '20')]
        value = int(split_str[0][0],16)
        error = int(split_str[0][1], 16)
        print(f"{split_str=}")
        print(f"{value=} {error=}")
        print(f"{ProtoError(error).name=}")

        
    logger.debug(f"<- '{str_data=}'")
    return str_data

def wait_for_response(ser:serial.Serial, value_to_wait_for:int|None = None, invalid_time_s:float=0.01) -> dict:
    starting_time = time.time()

    while((time.time()-starting_time) < invalid_time_s):
        data = ser.read_until(serial_delimiter)
        if len(data) == 0:
            continue
        logger.getChild("wfr").getChild("<-").debug(f"{data=}")
        str_data = data.decode().strip()
        try:
            jdata = json.loads(str_data)
        except json.JSONDecodeError as e:
            logger.error(f"{str_data=} IS NOT VALID JSON:{e=}")
            continue

        # logger.getChild("wfr").info(f"VALID JSON!")
        
        # if str_data.startswith("Value:"):
            # split_str = re.findall(r"Value: (?P<value>\w+), Error: (?P<error>\w+)", str_data)
            # split_str=[('002A', '20')]
        if 'value' not in jdata or 'error' not in jdata:
            continue
        value = jdata['value']
        error = jdata['error']
        # error = int(split_str[0][1], 16)
        protoerror = ProtoError(error)
        jdata['error'] = protoerror

        if protoerror != ProtoError.OK:
            logger.getChild("wfr").debug(f"returning {jdata}")
            return jdata
            return {"value":value, "error":protoerror}

        if value_to_wait_for is not None:
            if value_to_wait_for == value:
                logger.getChild("wfr").debug(f"returning {jdata}")
                return jdata
                return {"value":value, "error":ProtoError(error)}
            else:
                continue
        else:
            logger.getChild("wfr").debug(f"returning {jdata}")
            return jdata
            return {"value":value, "error":ProtoError(error)}
    raise ValueError("Did not receive the right data in time")


def test_setting_up_config(ser:serial.Serial):
    '''
    Test Steps:
    1. Make sure I am getting something from the serial port
    2. Send the testing command
    3. Read back the value
    '''
    
    # Test that I can echo back a command
    test_value = 0x3456
    send_command(ser, id=Commands.CONFIG_SET, param1=ConfigIndex.echo, param2=test_value, param3=0x7890)
    response = wait_for_response(ser)
    logger.info(f"{response=}")
    assert(response["error"] == ProtoError.OK)
    assert(response["value"] == test_value)

    # make sure that something that is out of range responds as expexted 
    send_command(ser, id=Commands.CONFIG_SET, param1=ConfigIndex.running, param2=test_value, param3=0x7890)
    response = wait_for_response(ser)
    logger.info(f"{response=}")
    assert(response["error"] == ProtoError.OUT_OF_RANGE)
    assert(response["value"] == ConfigIndex.running.value)

    # make sure that you can set a variable as expexted 
    send_command(ser, id=Commands.CONFIG_SET, param1=ConfigIndex.frame_count, param2=test_value, param3=0x0)
    response = wait_for_response(ser)
    logger.info(f"{response=}")
    assert(response["error"] == ProtoError.OK)
    assert(response["value"] == test_value)

    send_command(ser, id=Commands.CONFIG_GET, param1=ConfigIndex.frame_count, param2=test_value, param3=0x0)
    response = wait_for_response(ser)
    logger.info(f"{response=}")
    assert(response["error"] == ProtoError.OK)
    assert(response["value"] == test_value)

    # make sure that variable packet length works
    send_command(ser, id=Commands.NOOP, param1=ConfigIndex.echo)
    response = wait_for_response(ser)
    logger.info(f"{response=}")
    assert(response["error"] == ProtoError.OK)
    assert(response["value"] == 0)


def rgb_to_int(red:int, green:int, blue:int) -> int:
    return (red<<16)|(green<<8)|blue

def shift_fix(input:int) -> int:
    return input<<8


def send_and_check_command(ser:serial.Serial, id:CommandValue, param1:CommandValue, param2:CommandValue = -1, param3:CommandValue = -1):
    send_command(ser, id=id, param1=param1, param2=param2, param3=param3)
    response = wait_for_response(ser)
    if response["error"] != ProtoError.OK:
        logger.error(f"{response}")
        raise ValueError()
    send_command(ser, id=id, param1=param1, param2=param2, param3=param3)
    response = wait_for_response(ser)
    if response["error"] != ProtoError.OK:
        logger.error(f"{response}")
        raise ValueError()
    # TODO: add something that checks to see if its param1, 2 or 3
    return response["value"]


def set_config(ser:serial.Serial, config_index:ConfigIndex|int, config_value:int):
    if isinstance(config_index, Enum):
        config_index = config_index.value

    while True:
        try:
            send_command(ser, id=Commands.CONFIG_SET, data=[config_index, config_value])
            response = wait_for_response(ser, config_index)
            if response["error"] != ProtoError.OK:
                logger.getChild("set_config").error(f"{response}")
            # if response['value'] !=  Commands.config_index:
            #     logger.getChild("set_config").error(f"got response but not for config index {config_index:X}")
            #     continue
        except ValueError:
            logger.getChild("set_config").info(f"TIMEOUT on receiving data. Trying again.")
            continue
        break
    

def get_config(ser:serial.Serial, config_index:ConfigIndex|int) -> int|None:
    if isinstance(config_index, Enum):
        config_index = config_index.value
    while True:
        try:
            send_command(ser, id=Commands.CONFIG_GET, data=[config_index])
            response = wait_for_response(ser)
            if response["error"] != ProtoError.OK:
                logger.getChild("get_config").error(f"{response}")
            return response.get('value',None)
        except ValueError:
            logger.getChild("get_config").info(f"TIMEOUT on receiving data. Trying again.")
            continue
        break
    return None


def set_color(ser:serial.Serial, frame_index:int, led_index:int, led_color:int):

    while True:
        try:
            send_command(ser, id=Commands.COLOR_SET, data=[frame_index, led_index, led_color])

            response = wait_for_response(ser)
            if response["error"] != ProtoError.OK:
                logger.getChild("set_color").error(f"{response} {frame_index=} {led_index=}")
        except ValueError:
            logger.getChild("set_color").info(f"TIMEOUT on receiving data. Trying again.")
            continue
        break

def set_color_array(ser:serial.Serial, frame_index:int, color_array:list[int]):
    chunk_size = 60
    # chunks = np.array_split(color_array, chunk_size)
    it = iter(color_array)
    chunks =[list(islice(it, chunk_size)) for _ in range((len(color_array) + chunk_size - 1) // chunk_size)]
    for chunk_index, chunk in enumerate(chunks):
        while True:
            try:
                send_command(ser, id=Commands.MULTI_COLOR_SET, data=[frame_index, chunk_index*chunk_size, *chunk])

                response = wait_for_response(ser)
                if response["error"] != ProtoError.OK:
                    logger.getChild("set_color_array").error(f"{response} {frame_index=} {chunk=}")
            except ValueError:
                logger.getChild("set_color_array").info(f"TIMEOUT on receiving data. Trying again.")
                continue
            break

def compact_file(color_array:list[int]) -> list[int]:
    result = []
    counting = []
    count = 1
    working_value = -1
    n = len(color_array)
    result.append(color_array[0])
    counting.append(0)
    for index,value in enumerate(color_array):
        if value == result[-1]:
            counting[-1] += 1
        else:
            result.append(value)
            counting.append(1)
    final_result = []

    for index,value in enumerate(result):
        final_result.append(value + counting[index])
    return final_result


def set_color_array_file(ser:serial.Serial, file_id:int, starting_locaiton:int, update:int, color_array:list[int]):
    chunk_size = 55
    # chunks = np.array_split(color_array, chunk_size)
    it = iter(color_array)
    chunks =[list(islice(it, chunk_size)) for _ in range((len(color_array) + chunk_size - 1) // chunk_size)]
    for chunk_index, chunk in enumerate(chunks):
        while True:
            try:
                send_command(ser, id=Commands.FILE_SET, data=[file_id, starting_locaiton,  update, *chunk])

                response = wait_for_response(ser)
                if response["error"] != ProtoError.OK:
                    logger.getChild("set_color_array").error(f"{response} {file_id=} {chunk=}")
            except ValueError:
                logger.getChild("set_color_array").info(f"TIMEOUT on receiving data. Trying again.")
                continue
            break
        # current_location += len(chunk)


def send_lighting_frames(ser:serial.Serial):

    # turn on the debug printing so I can see whats going on
    set_config(ser,ConfigIndex.debug_cmd, 0x0)
    
    # turn off the reporting as it can slow_down/error out the bulk of commands
    set_config(ser,ConfigIndex.status_report, 0x0)
    
    

    # set up number of LED's
    desired_leds = 100
    set_config(ser,ConfigIndex.led_count, desired_leds)
   
    
    # set up number of frames
    desired_frames = 100
    set_config(ser,ConfigIndex.frame_count, desired_frames)

    
    # set up the FPS_ms
    desired_fps = 10
    fps_ms = int(round(1000.0/desired_fps,0))
    logger.info(f"Actual FPS: {fps_ms=}")
    set_config(ser,ConfigIndex.fps_ms, fps_ms)

    get_config(ser, ConfigIndex.fps_ms)
    
    send_frames = True
    if send_frames:
        # color in the first frame
        (rb, gb, bb) = (255,145, 145) #base color
        (rf, gf, bf) = (0, 0 ,0) # color to fade to
        base_color = shift_fix(rgb_to_int(rb,gb,bb))
        fade_amount = 20
        red_linspace = np.linspace(rb,rf,fade_amount).astype(int).tolist()
        green_linspace = np.linspace(gb,gf,fade_amount).astype(int).tolist()
        blue_linspace = np.linspace(bb,bf,fade_amount).astype(int).tolist()

        color_array:list[int] = [base_color]*desired_leds

        for index, (r,g,b) in enumerate(list(zip(red_linspace, green_linspace, blue_linspace))): # type: ignore
            color_array[index] = shift_fix(rgb_to_int(r,g,b)) 

        
        
        set_config(ser,ConfigIndex.running, 0x0)
        set_config(ser,ConfigIndex.debug_cmd, 0x0)

        file_id = 2
        starting_position = 3
        position = 0
        start_time = time.time()
        for frame_index in range(0,desired_frames+1):
            compact_array = compact_file(color_array)
            # set_color_array(ser, frame_index, color_array)
            # if frame_index == 97:
            #     print("stop here")
            #     compact_array = compact_file(color_array)
            

            set_color_array_file(ser, file_id, starting_position, position, compact_array)
            position = 1
    
            color_array = np.roll(color_array,1,axis=0).astype(int).tolist() # type: ignore
            logger.getChild("Frame Generation").info(f"{(frame_index*100)/desired_frames:0.2f}% done")
        
        set_config(ser,ConfigIndex.status_report, 0x1)
        set_config(ser, ConfigIndex.running, 0x1)
        set_config(ser, ConfigIndex.current_file, file_id)
        result= get_config(ser, ConfigIndex.status_report)
        logger.info(f"Status Report Value: {result}")
        end_time = time.time()
        logger.info(f"It took {end_time-start_time:0.2f} seconds to send {desired_leds} lights with {desired_frames} frames")

        
        
    # turn on the reporting
    set_config(ser, ConfigIndex.status_report, 0x1)
    # send_command(ser, id=Commands.CONFIG_SET, data=[ConfigIndex.status_report, 0x1])


def main():
     # Adjust your serial port and baud rate
    ser = serial.Serial('/dev/ttyACM0', 115200, timeout=0.1)
    time.sleep(0.2)  # wait for rp2040 to reset

    # default_sleep_time = 1
    # send_command(ser, id=Commands.CONFIG_SET, data=[ConfigIndex.frame_count, 1])
    # time.sleep(default_sleep_time)
    # send_command(ser, id=Commands.CONFIG_SET, data=[ConfigIndex.led_count, 100])
    # time.sleep(default_sleep_time)

    # send_command(ser, id=Commands.COLOR_SET, data=[0x0, 0x0, shift_fix(rgb_to_int(0,255,0))])
    # time.sleep(default_sleep_time)
    # send_command(ser, id=Commands.COLOR_GET, data=[0x0, 0x0])
    # time.sleep(default_sleep_time)
    # send_command(ser, id=Commands.MULTI_COLOR_SET, data=[0x0, 0x0, 
    #                                                      shift_fix(rgb_to_int(0,255,0)),
    #                                                      shift_fix(rgb_to_int(0,255,0)),
    #                                                      shift_fix(rgb_to_int(0,255,0)),
    #                                                      shift_fix(rgb_to_int(0,255,0)),
    #                                                      shift_fix(rgb_to_int(0,255,0)),
    #                                                      shift_fix(rgb_to_int(0,255,0)),
    #                                                      shift_fix(rgb_to_int(0,255,0)),
    #                                                      shift_fix(rgb_to_int(0,255,0)),
    #                                                      shift_fix(rgb_to_int(0,255,0)),
    #                                                      shift_fix(rgb_to_int(0,255,0)),
    #                                                      shift_fix(rgb_to_int(0,255,0)),
    #                                                      shift_fix(rgb_to_int(0,255,0)),
    #                                                      shift_fix(rgb_to_int(0,255,0)),
    #                                                      shift_fix(rgb_to_int(0,255,0)),
    #                                                      shift_fix(rgb_to_int(0,255,0)),
    #                                                      shift_fix(rgb_to_int(0,255,0))
    #                                                      ])
    # time.sleep(default_sleep_time)

    # send_command(ser, id=Commands.NOOP, data=[])
    # time.sleep(default_sleep_time)

    send_lighting_frames(ser)
    
    

    print("Command sent!")
    last_command_sent = time.time()
    while (time.time() - last_command_sent < 3.0):
        read_data = read_command(ser)
        logger.info(f"{read_data=}")

# Example usage
if __name__ == "__main__":
   logging.basicConfig(level=logging.INFO)
   main()
