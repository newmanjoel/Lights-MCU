import serial
import struct
import time

# Define struct format:
# <  = little-endian
# B  = uint8_t
# H  = uint16_t
# Structure: id, param1, param2, param3
PACKET_FORMAT = "<BBHH"
serial_delimiter = b'\n'

def send_command(ser, id, param1, param2, param3):
    """Send a Command struct over Serial in binary format."""
    packet = struct.pack(PACKET_FORMAT, id, param1, param2, param3)
    ser.write(packet + serial_delimiter)
    ser.flush()  # ensure all bytes are sent
    print(f"'{packet=}'")

def read_command(ser, id, timeout=1.0):
    ser.timeout = timeout
    data = ser.read_until(serial_delimiter)
    print(f"'{data=}'")

# Example usage
if __name__ == "__main__":
    # Adjust your serial port and baud rate
    ser = serial.Serial('/dev/ttyACM0', 115200, timeout=1)
    time.sleep(0.2)  # wait for Arduino to reset

    # Example: send a command
    # command 1 is command set
    # command 2 is command get
    send_command(ser, id=1, param1=42, param2=0, param3=0)
    read_command(ser, id=1)
    print("Command sent!")
