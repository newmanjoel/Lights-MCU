#pragma once

#ifndef CONSTANTS_H
#define CONSTANTS_H
    #include <cstdint>
    constexpr uint8_t max_frame_len = 1;
    constexpr uint8_t max_led_len = 250;
    constexpr uint8_t uart_buffer_len = 255;
    constexpr uint32_t max_data_len = 3000;
    // 20_000 is ok

    constexpr char START_CONDITION = 0xAA;
    constexpr char END_CONDITION = 0x55;
    constexpr uint8_t CRC_LEN = 0x01;
    constexpr uint8_t HEADER_LEN = 0x03;

    // the amount of time that has passed that the message should be considered invalid
    // 100000 seems to work?
    constexpr uint32_t uart_invalid_timeout_us = 100000;

    

#endif // CONSTANTS_H