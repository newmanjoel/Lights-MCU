from enum import Enum
import serial
import struct
import time
import re
import numpy as np
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


def send_command(ser:serial.Serial, id:CommandValue, param1:CommandValue, param2:CommandValue = -1, param3:CommandValue = -1):
    """Send a Command struct over Serial in binary format."""
    if isinstance(id, Enum):
        id = id.value
    if isinstance(param1, Enum):
        param1 = param1.value
    if isinstance(param2, Enum):
        param2 = param2.value
    if isinstance(param3, Enum):
        param3 = param3.value
    
    assert(type(id) is int)
    assert(type(param1) is int)
    assert(type(param2) is int)

    # assert(type(param3) is int)

    packet_format = PACKET_FORMAT
    if param2 == -1 and param3 == -1:
        packet_format = ">5BBB"
        logger.debug(f"Packet format: {packet_format}")
        packet = struct.pack(packet_format, 
            start_packet, 
            version,
            id, 
            0x01, # length in bytes
            param1,
            0xFF, # CRC
            end_packet
        )
    else:
        packet_format = ">5BHIBB"
        packet = struct.pack(packet_format, 
            start_packet, 
            version,
            id, 
            0x07, # length in bytes
            param1, #1 B
            param2, #2 B
            param3, #4 B
            0xFF, # CRC
            end_packet
        )
    
    result = ser.write(packet)
    ser.flush()  # ensure all bytes are sent
    logger.debug(f"-> {result} '{packet.hex(":")=}'")

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

def wait_for_response(ser:serial.Serial, value_to_wait_for:int|None = None, invalid_time_s:float=2.0) -> dict:
    starting_time = time.time()

    while((time.time()-starting_time) < invalid_time_s):
        data = ser.read_until(serial_delimiter)
        logger.getChild("serial").debug(f"{data=}")
        str_data = data.decode().strip()
        if str_data.startswith("Value:"):
            split_str = re.findall(r"Value: (?P<value>\w+), Error: (?P<error>\w+)", str_data)
            # split_str=[('002A', '20')]
            value = int(split_str[0][0],16)
            error = int(split_str[0][1], 16)

            if value_to_wait_for is not None:
                if value_to_wait_for == value:
                    return {"value":value, "error":ProtoError(error)}
                else:
                    pass
            else:
                return {"value":value, "error":ProtoError(error)}
    raise ValueError("Did not receive the right data in time")
    return {"value":None, "error":None}


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




def main():
     # Adjust your serial port and baud rate
    ser = serial.Serial('/dev/ttyACM0', 115200, timeout=1.5)
    time.sleep(0.2)  # wait for Arduino to reset

    # system_connunication = False
    
    # while not system_connunication:
    #     read_data = read_command(ser)
    #     if read_data.startswith('Current State:00'):
    #         system_connunication = True
    
    # test_setting_up_config(ser)

    # turn off the reporting as it can slow_down/error out the bulk of commands
    send_command(ser, id=Commands.CONFIG_SET, param1=ConfigIndex.status_report, param2=0x0, param3=0x0)
    response = wait_for_response(ser)
    if response["error"] != ProtoError.OK:
        logger.error(f"{response}")
        return

    # set up number of LED's
    desired_leds = 100
    send_command(ser, id=Commands.CONFIG_SET, param1=ConfigIndex.led_count, param2=desired_leds, param3=0x0)
    response = wait_for_response(ser, Commands.CONFIG_SET.value)
    if response["error"] != ProtoError.OK:
        logger.error(f"{response}")
        return
    
    # set up number of frames
    desired_frames = 50
    send_command(ser, id=Commands.CONFIG_SET, param1=ConfigIndex.frame_count, param2=desired_frames, param3=0x0)
    response = wait_for_response(ser)
    if response["error"] != ProtoError.OK:
        logger.error(f"{response}")
        return
    
    # set up the FPS_ms
    desired_fps = 30
    fps_ms = int(round(1000.0/desired_fps,0))
    logger.info(f"Actual FPS: {fps_ms=}")
    send_command(ser, id=Commands.CONFIG_SET, param1=ConfigIndex.fps, param2=fps_ms, param3=0x0)
    response = wait_for_response(ser)
    if response["error"] != ProtoError.OK:
        logger.error(f"{response}")
        return
    send_command(ser, id=Commands.CONFIG_GET, param1=ConfigIndex.fps, param2=0x00, param3=0x0)
    response = wait_for_response(ser)
    assert(response["value"] == fps_ms)
    if response["error"] != ProtoError.OK:
        logger.error(f"{response}")
        return
    
    send_frames = True
    if send_frames:
        # color in the first frame
        (rb, gb, bb) = (145,145, 145) #base color
        (rf, gf, bf) = (0, 0 ,0) # color to fade to
        base_color = shift_fix(rgb_to_int(rb,gb,bb))
        fade_amount = 20
        red_linspace = np.linspace(rb,rf,fade_amount).astype(int).tolist()
        green_linspace = np.linspace(gb,gf,fade_amount).astype(int).tolist()
        blue_linspace = np.linspace(bb,bf,fade_amount).astype(int).tolist()

        color_array = [base_color]*desired_leds

        for index, (r,g,b) in enumerate(list(zip(red_linspace, green_linspace, blue_linspace))):
            color_array[index] = shift_fix(rgb_to_int(r,g,b)) 
        
        send_command(ser, id=Commands.CONFIG_SET, param1=ConfigIndex.running, param2=0x0, param3=0x0)
        response = wait_for_response(ser)
        if response["error"] != ProtoError.OK:
            logger.error(f"{response}")
            return
        
        send_command(ser, id=Commands.CONFIG_SET, param1=ConfigIndex.debug_cmd, param2=0x0, param3=0x0)
        if response["error"] != ProtoError.OK:
            logger.error(f"{response}")
            return
        
        start_time = time.time()
        for frame_index in range(0,desired_frames+1):
            for led_index, led_color in enumerate(color_array):
                if led_index == 0:
                    led_color = shift_fix(rgb_to_int(158,64,13)) 
                send_command(ser, id=Commands.COLOR_SET, param1=frame_index, param2=led_index, param3=led_color)
                response = wait_for_response(ser)
                if response["error"] != ProtoError.OK:
                    logger.error(f"{response} {frame_index=} {led_index=}")
                
            color_array = np.roll(color_array,1,axis=0).astype(int).tolist()
            logger.getChild("Frame Generation").info(f"{(frame_index*100)/desired_frames:0.2f}% done")
        
        send_command(ser, id=Commands.CONFIG_SET, param1=ConfigIndex.running, param2=0x1, param3=0x0)
        response = wait_for_response(ser)
        if response["error"] != ProtoError.OK:
            logger.error(f"{response}")
            return
        end_time = time.time()
        logger.info(f"It took {end_time-start_time:0.2f} seconds to send {desired_leds} lights with {desired_frames} frames")

        
        
    # turn on the reporting
    send_command(ser, id=Commands.CONFIG_SET, param1=ConfigIndex.status_report, param2=0x1, param3=0x0)

    print("Command sent!")
    last_command_sent = time.time()
    while (time.time() - last_command_sent < 3.0):
        read_data = read_command(ser)
        logger.info(f"{read_data=}")

# Example usage
if __name__ == "__main__":
   logging.basicConfig(level=logging.INFO)
   main()
