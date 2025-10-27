#ifndef NRF24L01P_H
#define NRF24L01P_H

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <stdio.h>
#include <string>
#include "registers.h"


enum class Baudrate : uint8_t{
    OneMeg = 0x00,
    TwoMeg = 0x08,
    TwoFiftyKilo = 0x20,
};

struct NRF_HAL{
    uint8_t sck; // serial clock
    uint8_t mosi; // master out slave in
    uint8_t miso; // master in slave out
    uint8_t ce; // chip enable
    uint8_t csn; // chip select
    uint8_t irq; // attached to interrupt
};

class NRF24{
    
    public:
        uint8_t rx_reg[6];
        uint8_t tx_reg[6];
        NRF_HAL pinout;
        spi_inst* spi;
        uint8_t device_address;
        NRF24_Registers::CONFIG status;
        void init(NRF_HAL pinout, spi_inst* spi_instance, uint8_t device_address);
        void enable();
        void disable();
        void start_transaction();
        void stop_transaction();

        bool ChipAvaliable();
        void WriteReg(NRF24_Registers::Register reg, uint8_t value);
        void WriteReg(uint8_t reg, uint8_t value);
        uint8_t ReadReg(NRF24_Registers::Register reg);
        uint8_t ReadReg(uint8_t reg);
        void GetState();

        void SetPowerDownMode();
        void SetStandbyMode();
        void SetRXMode();
        void SetTXMode();




};


#endif
