from enum import Enum


class ProtoError(Enum):
    OK = 0x00
    # Protocol-level issues
    BAD_HEADER        = 0x10
    BAD_CHECKSUM      = 0x11
    UNEXPECTED_TYPE   = 0x12
    PAYLOAD_TOO_LONG  = 0x13
    BAD_VERSION       = 0x14
    BAD_COMMAND       = 0x15 

    # Parameter / data issues
    INVALID_PARAM     = 0x20
    OUT_OF_RANGE      = 0x21
    MISSING_FIELD     = 0x22

    # Internal / runtime
    TIMEOUT           = 0x30
    BUFFER_OVERFLOW   = 0x31
    UNKNOWN_ERROR     = 0xFF

class Commands(Enum):
    NOOP = 0x00
    START = 0x01
    STOP = 0x02
    CONFIG_SET = 0x03
    CONFIG_GET = 0x04
    COLOR_SET = 0x05
    COLOR_GET = 0x06

class ConfigIndex(Enum):
    echo = 0x00
    fps = 0x01
    running = 0x02
    led_count = 0x03
    frame_count = 0x04
    debug_r = 0x05
    debug_g = 0x06
    debug_b = 0x07
    