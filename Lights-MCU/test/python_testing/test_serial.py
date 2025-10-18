from enum import Enum
import serial
import struct
import time
import re
import structs
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
    assert(type(param3) is int)

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
        
        packet = struct.pack(packet_format, 
            start_packet, 
            version,
            id, 
            0x05, # length in bytes
            param1, #1 B
            param2, #2 B
            param3, #2 B
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

def wait_for_response(ser:serial.Serial, invalid_time_s:float=2.0) -> dict:
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
            logger.debug(f"{split_str=}")
            logger.debug(f"{value=} {error=}")
            logger.debug(f"{ProtoError(error).name=}")
            return {"value":value, "error":ProtoError(error)}
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




def main():
     # Adjust your serial port and baud rate
    ser = serial.Serial('/dev/ttyACM0', 115200, timeout=1.5)
    time.sleep(0.2)  # wait for Arduino to reset

    system_connunication = False
    
    while not system_connunication:
        read_data = read_command(ser)
        if read_data == 'Current State:00':
            system_connunication = True
    
    test_setting_up_config(ser)


    send_command(ser, id=Commands.CONFIG_SET.value, param1=ConfigIndex.echo.value, param2=0x3456, param3=0x7890)
    print("Command sent!")

    system_connunication = False
    while not system_connunication:
        read_data = read_command(ser)
        if read_data == 'Current State:00':
            system_connunication = True

# Example usage
if __name__ == "__main__":
   logging.basicConfig(level=logging.DEBUG)
   main()
