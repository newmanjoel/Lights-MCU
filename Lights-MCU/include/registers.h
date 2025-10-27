#pragma once
#include <stdint.h>

// Auto-generated register definitions

namespace NRF24_Registers{
    // CONFIG (00): Configuration Register
    // Type: R/W, Width: 8 bits, Reset: 00
    struct CONFIG {
        uint8_t Reserved    : 1;     // Only '0' allowed
        uint8_t MASK_RX_DR  : 1;     // Mask interrupt caused by RX_DR 1: Interrupt not reflected on the IRQ pin 0: Reflect RX_DR as active low interrupt on the IRQ pin
        uint8_t MASK_TX_DS  : 1;     // Mask interrupt caused by TX_DS 1: Interrupt not reflected on the IRQ pin 0: Reflect TX_DS as active low interrupt on the IRQ pin
        uint8_t MASK_MAX_RT : 1;     // Mask interrupt caused by MAX_RT 1: Interrupt not reflected on the IRQ pin 0: Reflect MAX_RT as active low interrupt on the IRQ pin
        uint8_t EN_CRC      : 1;     // Enable CRC. Forced high if one of the bits in the EN_AA is high
        uint8_t CRCO        : 1;     // CRC encoding scheme '0' - 1 byte '1' – 2 bytes
        uint8_t PWR_UP      : 1;     // 1: POWER UP, 0:POWER DOWN
        uint8_t PRIM_RX     : 1;     // RX/TX control 1: PRX, 0: PTX

        CONFIG& operator=(uint8_t n) {
            Reserved    = (n >> 7) & 0x01;
            MASK_RX_DR  = (n >> 6) & 0x01;
            MASK_TX_DS  = (n >> 5) & 0x01;
            MASK_MAX_RT = (n >> 4) & 0x01;
            EN_CRC      = (n >> 3) & 0x01;
            CRCO        = (n >> 2) & 0x01;
            PWR_UP      = (n >> 1) & 0x01;
            PRIM_RX     = (n >> 0) & 0x01;
            return *this;
        };

        uint8_t to_uint8_t() const {
            uint8_t n = 0;
            n |= (Reserved    & 0x01) << 7;
            n |= (MASK_RX_DR  & 0x01) << 6;
            n |= (MASK_TX_DS  & 0x01) << 5;
            n |= (MASK_MAX_RT & 0x01) << 4;
            n |= (EN_CRC      & 0x01) << 3;
            n |= (CRCO        & 0x01) << 2;
            n |= (PWR_UP      & 0x01) << 1;
            n |= (PRIM_RX     & 0x01) << 0;
            return n;
        };
    };

    // EN_AA (01): Enable ‘Auto Acknowledgment’ Function Disable this functionality to be compatible with nRF2401, see page 75
    // Type: R/W, Width: 8 bits, Reset: 3F
    struct EN_AA {
        uint8_t Reserved : 2;     // Only '00' allowed
        uint8_t ENAA_P5  : 1;     // Enable auto acknowledgement data pipe 5
        uint8_t ENAA_P4  : 1;     // Enable auto acknowledgement data pipe 4
        uint8_t ENAA_P3  : 1;     // Enable auto acknowledgement data pipe 3
        uint8_t ENAA_P2  : 1;     // Enable auto acknowledgement data pipe 2
        uint8_t ENAA_P1  : 1;     // Enable auto acknowledgement data pipe 1
        uint8_t ENAA_P0  : 1;     // Enable auto acknowledgement data pipe 0

        EN_AA& operator=(uint8_t n) {
            Reserved = (n >> 6) & 0x03;
            ENAA_P5  = (n >> 5) & 0x01;
            ENAA_P4  = (n >> 4) & 0x01;
            ENAA_P3  = (n >> 3) & 0x01;
            ENAA_P2  = (n >> 2) & 0x01;
            ENAA_P1  = (n >> 1) & 0x01;
            ENAA_P0  = (n >> 0) & 0x01;
            return *this;
        };

        uint8_t to_uint8_t() const {
            uint8_t n = 0;
            n |= (Reserved & 0x03) << 6;
            n |= (ENAA_P5  & 0x01) << 5;
            n |= (ENAA_P4  & 0x01) << 4;
            n |= (ENAA_P3  & 0x01) << 3;
            n |= (ENAA_P2  & 0x01) << 2;
            n |= (ENAA_P1  & 0x01) << 1;
            n |= (ENAA_P0  & 0x01) << 0;
            return n;
        };
    };

    // EN_RXADDR (02): Enabled RX Addresses
    // Type: R/W, Width: 8 bits, Reset: 03
    struct EN_RXADDR {
        uint8_t Reserved : 2;     // Only '00' allowed
        uint8_t ERX_P5   : 1;     // Enable data pipe 5
        uint8_t ERX_P4   : 1;     // Enable data pipe 4
        uint8_t ERX_P3   : 1;     // Enable data pipe 3
        uint8_t ERX_P2   : 1;     // Enable data pipe 2
        uint8_t ERX_P1   : 1;     // Enable data pipe 1
        uint8_t ERX_P0   : 1;     // Enable data pipe 0

        EN_RXADDR& operator=(uint8_t n) {
            Reserved = (n >> 6) & 0x03;
            ERX_P5   = (n >> 5) & 0x01;
            ERX_P4   = (n >> 4) & 0x01;
            ERX_P3   = (n >> 3) & 0x01;
            ERX_P2   = (n >> 2) & 0x01;
            ERX_P1   = (n >> 1) & 0x01;
            ERX_P0   = (n >> 0) & 0x01;
            return *this;
        };

        uint8_t to_uint8_t() const {
            uint8_t n = 0;
            n |= (Reserved & 0x03) << 6;
            n |= (ERX_P5   & 0x01) << 5;
            n |= (ERX_P4   & 0x01) << 4;
            n |= (ERX_P3   & 0x01) << 3;
            n |= (ERX_P2   & 0x01) << 2;
            n |= (ERX_P1   & 0x01) << 1;
            n |= (ERX_P0   & 0x01) << 0;
            return n;
        };
    };

    // SETUP_AW (03): Setup of Address Widths (common for all data pipes)
    // Type: R/W, Width: 8 bits, Reset: 03
    struct SETUP_AW {
        uint8_t Reserved : 6;     // Only '00' allowed
        uint8_t AW       : 2;     // RX/TX Address field width '00' - Illegal '01' - 3 bytes '10' - 4 bytes '11' – 5 bytes LSByte is used if address width is below 5 bytes

        SETUP_AW& operator=(uint8_t n) {
            Reserved = (n >> 2) & 0x3F;
            AW       = (n >> 0) & 0x03;
            return *this;
        };

        uint8_t to_uint8_t() const {
            uint8_t n = 0;
            n |= (Reserved & 0x3F) << 2;
            n |= (AW       & 0x03) << 0;
            return n;
        };
    };

    // SETUP_RETR (04): Setup of Automatic Retransmission
    // Type: R/W, Width: 8 bits, Reset: 03
    struct SETUP_RETR {
        uint8_t ARD : 4;     // Auto Retransmit Delay ‘0000’ – Wait 250µS ‘0001’ – Wait 500µS ‘0010’ – Wait 750µS …….. ‘1111’ – Wait 4000µS (Delay defined from end of transmission to start of next transmission)
        uint8_t ARC : 4;     // Auto Retransmit Count ‘0000’ –Re-Transmit disabled ‘0001’ – Up to 1 Re-Transmit on fail of AA …… ‘1111’ – Up to 15 Re-Transmit on fail of AA

        SETUP_RETR& operator=(uint8_t n) {
            ARD = (n >> 4) & 0x0F;
            ARC = (n >> 0) & 0x0F;
            return *this;
        };

        uint8_t to_uint8_t() const {
            uint8_t n = 0;
            n |= (ARD & 0x0F) << 4;
            n |= (ARC & 0x0F) << 0;
            return n;
        };
    };

    // RF_CHANNEL (05): RF Channel
    // Type: R/W, Width: 8 bits, Reset: 02
    struct RF_CHANNEL {
        uint8_t Reserved : 1;     // Only '0' allowed
        uint8_t RF_CH    : 7;     // Sets the frequency channel nRF24L01+ operates on

        RF_CHANNEL& operator=(uint8_t n) {
            Reserved = (n >> 7) & 0x01;
            RF_CH    = (n >> 0) & 0x7F;
            return *this;
        };

        uint8_t to_uint8_t() const {
            uint8_t n = 0;
            n |= (Reserved & 0x01) << 7;
            n |= (RF_CH    & 0x7F) << 0;
            return n;
        };
    };

    // RF_SETUP (06): RF Setup Register
    // Type: R/W, Width: 8 bits, Reset: 0E
    struct RF_SETUP {
        uint8_t CONT_WAVE  : 1;     // Enables continuous carrier transmit when high.
        uint8_t Reserved   : 1;     // Only '0' allowed
        uint8_t RF_DR_LOW  : 1;     // Set RF Data Rate to 250kbps. See RF_DR_HIGH for encoding.
        uint8_t PLL_LOCK   : 1;     // Force PLL lock signal. Only used in test
        uint8_t RF_DR_HIGH : 1;     // Select between the high speed data rates. This bit is don’t care if RF_DR_LOW is set. Encoding: [RF_DR_LOW, RF_DR_HIGH]: ‘00’ – 1Mbps ‘01’ – 2Mbps ‘10’ – 250kbps ‘11’ – Reserved
        uint8_t RF_PWR     : 2;     // Set RF output power in TX mode '00' – -18dBm '01' – -12dBm '10' – -6dBm '11' – 0dBm
        uint8_t Obsolete   : 1;     // Don’t care

        RF_SETUP& operator=(uint8_t n) {
            CONT_WAVE  = (n >> 7) & 0x01;
            Reserved   = (n >> 6) & 0x01;
            RF_DR_LOW  = (n >> 5) & 0x01;
            PLL_LOCK   = (n >> 4) & 0x01;
            RF_DR_HIGH = (n >> 3) & 0x01;
            RF_PWR     = (n >> 1) & 0x03;
            Obsolete   = (n >> 0) & 0x01;
            return *this;
        };

        uint8_t to_uint8_t() const {
            uint8_t n = 0;
            n |= (CONT_WAVE  & 0x01) << 7;
            n |= (Reserved   & 0x01) << 6;
            n |= (RF_DR_LOW  & 0x01) << 5;
            n |= (PLL_LOCK   & 0x01) << 4;
            n |= (RF_DR_HIGH & 0x01) << 3;
            n |= (RF_PWR     & 0x03) << 1;
            n |= (Obsolete   & 0x01) << 0;
            return n;
        };
    };

    // STATUS (07): Status Register (In parallel to the SPI command word applied on the MOSI pin, the STATUS register is shifted serially out on the MISO pin)
    // Type: Mixed, Width: 8 bits, Reset: 0E
    struct STATUS {
        uint8_t Reserved : 1;     // Only '0' allowed
        uint8_t RX_DR    : 1;     // Data Ready RX FIFO interrupt. Asserted when new data arrives RX FIFOc. Write 1 to clear bit.
        uint8_t TX_DS    : 1;     // Data Sent TX FIFO interrupt. Asserted when packet transmitted on TX. If AUTO_ACK is activated, this bit is set high only when ACK is received. Write 1 to clear bit.
        uint8_t MAX_RT   : 1;     // Maximum number of TX retransmits interrupt Write 1 to clear bit. If MAX_RT is asserted it must be cleared to enable further communication.
        uint8_t RX_P_NO  : 3;     // Data pipe number for the payload available for reading from RX_FIFO 000-101: Data Pipe Number 110: Not Used 111: RX FIFO Empty
        uint8_t TX_FULL  : 1;     // TX FIFO full flag. 1: TX FIFO full. 0: Available locations in TX FIFO.

        STATUS& operator=(uint8_t n) {
            Reserved = (n >> 7) & 0x01;
            RX_DR    = (n >> 6) & 0x01;
            TX_DS    = (n >> 5) & 0x01;
            MAX_RT   = (n >> 4) & 0x01;
            RX_P_NO  = (n >> 1) & 0x07;
            TX_FULL  = (n >> 0) & 0x01;
            return *this;
        };

        uint8_t to_uint8_t() const {
            uint8_t n = 0;
            n |= (Reserved & 0x01) << 7;
            n |= (RX_DR    & 0x01) << 6;
            n |= (TX_DS    & 0x01) << 5;
            n |= (MAX_RT   & 0x01) << 4;
            n |= (RX_P_NO  & 0x07) << 1;
            n |= (TX_FULL  & 0x01) << 0;
            return n;
        };
    };

    // OBSERVE_TX (08): Transmit observe register
    // Type: R, Width: 8 bits, Reset: 0
    struct OBSERVE_TX {
        uint8_t PLOS_CNT : 4;     // Count lost packets. The counter is overflow protected to 15, and discontinues at max until reset. The counter is reset by writing to RF_CH. See page 75.
        uint8_t ARC_CNT  : 4;     // Count retransmitted packets. The counter is reset when transmission of a new packet starts. See page 75.

        OBSERVE_TX& operator=(uint8_t n) {
            PLOS_CNT = (n >> 4) & 0x0F;
            ARC_CNT  = (n >> 0) & 0x0F;
            return *this;
        };

        uint8_t to_uint8_t() const {
            uint8_t n = 0;
            n |= (PLOS_CNT & 0x0F) << 4;
            n |= (ARC_CNT  & 0x0F) << 0;
            return n;
        };
    };

    // RX_PWR_D (09): Received Power Detector
    // Type: R, Width: 8 bits, Reset: 00
    struct RX_PWR_D {
        uint8_t Reserved : 7;     // nan
        uint8_t RPD      : 1;     // Received Power Detector. This register is called CD (Carrier Detect) in the nRF24L01. The name is different in nRF24L01+ due to the different input power level threshold for this bit. See section 6.4 on page 25.

        RX_PWR_D& operator=(uint8_t n) {
            Reserved = (n >> 1) & 0x7F;
            RPD      = (n >> 0) & 0x01;
            return *this;
        };

        uint8_t to_uint8_t() const {
            uint8_t n = 0;
            n |= (Reserved & 0x7F) << 1;
            n |= (RPD      & 0x01) << 0;
            return n;
        };
    };

    // Missing sheet for RX_ADDR_P0

    // Missing sheet for RX_ADDR_P1

    // Missing sheet for RX_ADDR_P2

    // Missing sheet for RX_ADDR_P3

    // Missing sheet for RX_ADDR_P4

    // Missing sheet for RX_ADDR_P5

    // Missing sheet for TX_ADDR

    // RX_PL_P0 (11): Bytes in RX payload in data pipe 0
    // Type: R/W, Width: 8 bits, Reset: 00
    struct RX_PL_P0 {
        uint8_t Reserved : 2;     // Only ‘00’ allowed
        uint8_t RX_PW_P0 : 6;     // Number of bytes in RX payload in data pipe 0 (1 to 32 bytes). 0 Pipe not used 1 = 1 byte … 32 = 32 bytes

        RX_PL_P0& operator=(uint8_t n) {
            Reserved = (n >> 6) & 0x03;
            RX_PW_P0 = (n >> 0) & 0x3F;
            return *this;
        };

        uint8_t to_uint8_t() const {
            uint8_t n = 0;
            n |= (Reserved & 0x03) << 6;
            n |= (RX_PW_P0 & 0x3F) << 0;
            return n;
        };
    };

    // RX_PL_P1 (12): Bytes in RX payload in data pipe 1
    // Type: R/W, Width: 8 bits, Reset: 00
    struct RX_PL_P1 {
        uint8_t Reserved : 2;     // Only ‘00’ allowed
        uint8_t RX_PW_P1 : 6;     // Number of bytes in RX payload in data pipe 1 (1 to 32 bytes). 0 Pipe not used 1 = 1 byte … 32 = 32 bytes

        RX_PL_P1& operator=(uint8_t n) {
            Reserved = (n >> 6) & 0x03;
            RX_PW_P1 = (n >> 0) & 0x3F;
            return *this;
        };

        uint8_t to_uint8_t() const {
            uint8_t n = 0;
            n |= (Reserved & 0x03) << 6;
            n |= (RX_PW_P1 & 0x3F) << 0;
            return n;
        };
    };

    // RX_PL_P2 (13): Bytes in RX payload in data pipe 2
    // Type: R/W, Width: 8 bits, Reset: 00
    struct RX_PL_P2 {
        uint8_t Reserved : 2;     // Only ‘00’ allowed
        uint8_t RX_PW_P2 : 6;     // Number of bytes in RX payload in data pipe 2 (1 to 32 bytes). 0 Pipe not used 1 = 1 byte … 32 = 32 bytes

        RX_PL_P2& operator=(uint8_t n) {
            Reserved = (n >> 6) & 0x03;
            RX_PW_P2 = (n >> 0) & 0x3F;
            return *this;
        };

        uint8_t to_uint8_t() const {
            uint8_t n = 0;
            n |= (Reserved & 0x03) << 6;
            n |= (RX_PW_P2 & 0x3F) << 0;
            return n;
        };
    };

    // RX_PL_P3 (14): Bytes in RX payload in data pipe 3
    // Type: R/W, Width: 8 bits, Reset: 00
    struct RX_PL_P3 {
        uint8_t Reserved : 2;     // Only ‘00’ allowed
        uint8_t RX_PW_P3 : 6;     // Number of bytes in RX payload in data pipe 3 (1 to 32 bytes). 0 Pipe not used 1 = 1 byte … 32 = 32 bytes

        RX_PL_P3& operator=(uint8_t n) {
            Reserved = (n >> 6) & 0x03;
            RX_PW_P3 = (n >> 0) & 0x3F;
            return *this;
        };

        uint8_t to_uint8_t() const {
            uint8_t n = 0;
            n |= (Reserved & 0x03) << 6;
            n |= (RX_PW_P3 & 0x3F) << 0;
            return n;
        };
    };

    // RX_PL_P4 (15): Bytes in RX payload in data pipe 4
    // Type: R/W, Width: 8 bits, Reset: 00
    struct RX_PL_P4 {
        uint8_t Reserved : 2;     // Only ‘00’ allowed
        uint8_t RX_PW_P4 : 6;     // Number of bytes in RX payload in data pipe 4 (1 to 32 bytes). 0 Pipe not used 1 = 1 byte … 32 = 32 bytes

        RX_PL_P4& operator=(uint8_t n) {
            Reserved = (n >> 6) & 0x03;
            RX_PW_P4 = (n >> 0) & 0x3F;
            return *this;
        };

        uint8_t to_uint8_t() const {
            uint8_t n = 0;
            n |= (Reserved & 0x03) << 6;
            n |= (RX_PW_P4 & 0x3F) << 0;
            return n;
        };
    };

    // RX_PL_P5 (16): Bytes in RX payload in data pipe 5
    // Type: R/W, Width: 8 bits, Reset: 00
    struct RX_PL_P5 {
        uint8_t Reserved : 2;     // Only ‘00’ allowed
        uint8_t RX_PW_P5 : 6;     // Number of bytes in RX payload in data pipe 5 (1 to 32 bytes). 0 Pipe not used 1 = 1 byte … 32 = 32 bytes

        RX_PL_P5& operator=(uint8_t n) {
            Reserved = (n >> 6) & 0x03;
            RX_PW_P5 = (n >> 0) & 0x3F;
            return *this;
        };

        uint8_t to_uint8_t() const {
            uint8_t n = 0;
            n |= (Reserved & 0x03) << 6;
            n |= (RX_PW_P5 & 0x3F) << 0;
            return n;
        };
    };

    // FIFO_STATUS (17): FIFO Status Register
    // Type: Mixed, Width: 8 bits, Reset: 01
    struct FIFO_STATUS {
        uint8_t Reserved_1 : 1;     // Only '0' allowed
        uint8_t TX_REUSE   : 1;     // Used for a PTX device Pulse the rfce high for at least 10µs to Reuse last transmitted payload. TX payload reuse is active until W_TX_PAYLOAD or FLUSH TX is executed. TX_REUSE is set by the SPI command REUSE_TX_PL, and is reset by the SPI commands W_TX_PAYLOAD or FLUSH TX
        uint8_t TX_FULL    : 1;     // TX FIFO full flag. 1: TX FIFO full. 0: Available locations in TX FIFO.
        uint8_t TX_EMPTY   : 1;     // TX FIFO empty flag. 1: TX FIFO empty. 0: Data in TX FIFO
        uint8_t Reserved_2 : 2;     // Only '00' allowed
        uint8_t RX_FULL    : 1;     // RX FIFO full flag. 1: RX FIFO full. 0: Available locations in RX FIFO
        uint8_t RX_EMPTY   : 1;     // RX FIFO empty flag. 1: RX FIFO empty. 0: Data in RX FIFO.

        FIFO_STATUS& operator=(uint8_t n) {
            Reserved_1 = (n >> 7) & 0x01;
            TX_REUSE   = (n >> 6) & 0x01;
            TX_FULL    = (n >> 5) & 0x01;
            TX_EMPTY   = (n >> 4) & 0x01;
            Reserved_2 = (n >> 2) & 0x03;
            RX_FULL    = (n >> 1) & 0x01;
            RX_EMPTY   = (n >> 0) & 0x01;
            return *this;
        };

        uint8_t to_uint8_t() const {
            uint8_t n = 0;
            n |= (Reserved_1 & 0x01) << 7;
            n |= (TX_REUSE   & 0x01) << 6;
            n |= (TX_FULL    & 0x01) << 5;
            n |= (TX_EMPTY   & 0x01) << 4;
            n |= (Reserved_2 & 0x03) << 2;
            n |= (RX_FULL    & 0x01) << 1;
            n |= (RX_EMPTY   & 0x01) << 0;
            return n;
        };
    };

    // DYNPD (1C): Enable dynamic payload length
    // Type: R/W, Width: 8 bits, Reset: 00
    struct DYNPD {
        uint8_t Reserved : 2;     // Only ‘00’ allowed
        uint8_t DPL_P5   : 1;     // Enable dynamic payload length data pipe 5. (Requires EN_DPL and ENAA_P5)
        uint8_t DPL_P4   : 1;     // Enable dynamic payload length data pipe 4. (Requires EN_DPL and ENAA_P4)
        uint8_t DPL_P3   : 1;     // Enable dynamic payload length data pipe 3. (Requires EN_DPL and ENAA_P3)
        uint8_t DPL_P2   : 1;     // Enable dynamic payload length data pipe 2. (Requires EN_DPL and ENAA_P2)
        uint8_t DPL_P1   : 1;     // Enable dynamic payload length data pipe 1. (Requires EN_DPL and ENAA_P1)
        uint8_t DPL_P0   : 1;     // Enable dynamic payload length data pipe 0. (Requires EN_DPL and ENAA_P0)

        DYNPD& operator=(uint8_t n) {
            Reserved = (n >> 6) & 0x03;
            DPL_P5   = (n >> 5) & 0x01;
            DPL_P4   = (n >> 4) & 0x01;
            DPL_P3   = (n >> 3) & 0x01;
            DPL_P2   = (n >> 2) & 0x01;
            DPL_P1   = (n >> 1) & 0x01;
            DPL_P0   = (n >> 0) & 0x01;
            return *this;
        };

        uint8_t to_uint8_t() const {
            uint8_t n = 0;
            n |= (Reserved & 0x03) << 6;
            n |= (DPL_P5   & 0x01) << 5;
            n |= (DPL_P4   & 0x01) << 4;
            n |= (DPL_P3   & 0x01) << 3;
            n |= (DPL_P2   & 0x01) << 2;
            n |= (DPL_P1   & 0x01) << 1;
            n |= (DPL_P0   & 0x01) << 0;
            return n;
        };
    };

    // FEATURE (1D): Feature Register
    // Type: R/W, Width: 8 bits, Reset: 00
    struct FEATURE {
        uint8_t Reserved   : 5;     // Only ‘00000’ allowed
        uint8_t EN_DPL     : 1;     // Enables Dynamic Payload Length
        uint8_t EN_ACK_PAY : 1;     // Enables Payload with ACK
        uint8_t EN_DYN_ACK : 1;     // Enables the W_TX_PAYLOAD_NOACK command

        FEATURE& operator=(uint8_t n) {
            Reserved   = (n >> 3) & 0x1F;
            EN_DPL     = (n >> 2) & 0x01;
            EN_ACK_PAY = (n >> 1) & 0x01;
            EN_DYN_ACK = (n >> 0) & 0x01;
            return *this;
        };

        uint8_t to_uint8_t() const {
            uint8_t n = 0;
            n |= (Reserved   & 0x1F) << 3;
            n |= (EN_DPL     & 0x01) << 2;
            n |= (EN_ACK_PAY & 0x01) << 1;
            n |= (EN_DYN_ACK & 0x01) << 0;
            return n;
        };
    };

    // Register map enum
    enum class Register : uint32_t {
        CONFIG      = 0x00, // Configuration Register
        EN_AA       = 0x01, // Enable ‘Auto Acknowledgment’ Function Disable this functionality to be compatible with nRF2401, see page 75
        EN_RXADDR   = 0x02, // Enabled RX Addresses
        SETUP_AW    = 0x03, // Setup of Address Widths (common for all data pipes)
        SETUP_RETR  = 0x04, // Setup of Automatic Retransmission
        RF_CHANNEL  = 0x05, // RF Channel
        RF_SETUP    = 0x06, // RF Setup Register
        STATUS      = 0x07, // Status Register (In parallel to the SPI command word applied on the MOSI pin, the STATUS register is shifted serially out on the MISO pin)
        OBSERVE_TX  = 0x08, // Transmit observe register
        RX_PWR_D    = 0x09, // Received Power Detector
        RX_ADDR_P0  = 0x0A, // Receive address data pipe 0. 5 Bytes maximum length. (LSByte is written first. Write the number of bytes defined by SETUP_AW)
        RX_ADDR_P1  = 0x0B, // Receive address data pipe 1. 5 Bytes maximum length. (LSByte is written first. Write the number of bytes defined by SETUP_AW)
        RX_ADDR_P2  = 0x0C, // Receive address data pipe 2. Only LSB. MSBytes are equal to RX_ADDR_P1[39:8]
        RX_ADDR_P3  = 0x0D, // Receive address data pipe 3. Only LSB. MSBytes are equal to RX_ADDR_P1[39:8]
        RX_ADDR_P4  = 0x0E, // Receive address data pipe 4. Only LSB. MSBytes are equal to RX_ADDR_P1[39:8]
        RX_ADDR_P5  = 0x0F, // Receive address data pipe 5. Only LSB. MSBytes are equal to RX_ADDR_P1[39:8]
        TX_ADDR     = 0x10, // Transmit address. Used for a PTX device only. (LSByte is written first) Set RX_ADDR_P0 equal to this address to handle automatic acknowledge if this is a PTX device with Enhanced ShockBurst™ enabled. See page 75.
        RX_PL_P0    = 0x11, // Bytes in RX payload in data pipe 0
        RX_PL_P1    = 0x12, // Bytes in RX payload in data pipe 1
        RX_PL_P2    = 0x13, // Bytes in RX payload in data pipe 2
        RX_PL_P3    = 0x14, // Bytes in RX payload in data pipe 3
        RX_PL_P4    = 0x15, // Bytes in RX payload in data pipe 4
        RX_PL_P5    = 0x16, // Bytes in RX payload in data pipe 5
        FIFO_STATUS = 0x17, // FIFO Status Register
        DYNPD       = 0x1C, // Enable dynamic payload length
        FEATURE     = 0x1D, // Feature Register
    };

    enum class Commands : uint8_t {
        R_REGISTER          = 0x00, // Read command and status registers. AAAAA = 5 bit Register Map Address
        W_REGISTER          = 0x20, // Write command and status registers. AAAAA = 5 bit Register Map Address Executable in power down or standby modes only
        R_RX_PAYLOAD        = 0x61, // Read RX-payload: 1 – 32 bytes. A read operation always starts at byte 0. Payload is deleted from FIFO after it is read. Used in RX mode. 
        W_TX_PAYLOAD        = 0xA0, // Write TX-payload: 1 – 32 bytes. A write operation always starts at byte 0 used in TX payload.
        FLUSH_TX            = 0xE1, // Flush TX FIFO, used in TX mode 
        FLUSH_RX            = 0xE2, // Flush RX FIFO, used in RX mode Should not be executed during transmission of acknowledge, that is, acknowledge package will not be completed.
        REUSE_TX_PL         = 0xE3, // Used for a PTX device Reuse last transmitted payload. TX payload reuse is active until W_TX_PAYLOAD or FLUSH TX is executed. TX payload reuse must not be activated or deactivated during package transmission.
        R_RX_PL_WID         = 0x60, // Read RX payload width for the top R_RX_PAYLOAD in the RX FIFO. Note: Flush RX FIFO if the read value is larger than 32 bytes.
        W_ACK_PAYLOAD       = 0xA8, // Used in RX mode. Write Payload to be transmitted together with ACK packet on PIPE PPP. (PPP valid in the range from 000 to 101). Maximum three ACK packet payloads can be pending. Payloads with same PPP are handled using first in - first out principle. Write payload: 1– 32 bytes. A write operation always starts at byte 0.
        W_TX_PAYLOAD_NO_ACK = 0xB0, // Used in TX mode. Disables AUTOACK on this specific packet.
        NOP                 = 0xFF, // No Operation. Might be used to read the STATUS register
    };
}; // namespace NRF24_Registers