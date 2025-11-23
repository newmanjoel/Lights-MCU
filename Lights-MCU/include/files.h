#ifndef FILES_H
#define FILES_H

#include <cstdint>

enum class EndAction : uint8_t{
    REPEAT = 0x00,
    STOP = 0x01,
    RUN_FILE = 0x03,
    FUNCTION = 0x04
};

struct File{
   uint16_t start; // starting index in the data array
   uint16_t end;   // last index of the file (if a length of 1, should be the same as start)
   EndAction action;
};


#endif // FILES_H