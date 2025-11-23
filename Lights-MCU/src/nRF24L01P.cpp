
#include "nRF24L01P.h"
#include <string>



using namespace NRF24_Registers;
extern mutex_t uart_mutex;

void NRF24::enable(){
    gpio_put(pinout.ce, true);
}
void NRF24::disable(){
    gpio_put(pinout.ce, false);
}
void NRF24::start_transaction(){
    gpio_put(pinout.csn, false);
}
void NRF24::stop_transaction(){
    gpio_put(pinout.csn, true);
}

void NRF24::init(NRF_HAL pinout, spi_inst* spi_instance, uint8_t device_address){
    this->pinout = pinout;
    this->spi = spi_instance;
    this->device_address = device_address;
    this->status = {0};
    // setup the HAL

    auto actual_baudrate = spi_init(this->spi, 1000*1000);

    gpio_init(pinout.csn);
    gpio_init(pinout.ce);
    gpio_set_function(pinout.sck, GPIO_FUNC_SPI);
    gpio_set_function(pinout.mosi, GPIO_FUNC_SPI);
    gpio_set_function(pinout.miso, GPIO_FUNC_SPI);
    // gpio_set_function(pinout.csn, GPIO_FUNC_SPI);

    
    gpio_set_dir(pinout.csn, true);
    gpio_set_function(pinout.csn, GPIO_FUNC_SIO);
    gpio_set_dir(pinout.ce, true);
    gpio_set_function(pinout.ce, GPIO_FUNC_SIO);
    // gpio_set_function(pinout.irq, GPIO_FUNC_SIO);

    spi_set_format(this->spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    
    stop_transaction();
    enable();

    // setup the radio so that its slow at first
    RF_SETUP rf_setup = {0};
    rf_setup = ReadReg(Register::RF_SETUP);
    rf_setup.RF_DR_LOW = 1;
    rf_setup.RF_DR_HIGH = 0;
    WriteReg(Register::RF_SETUP, rf_setup.to_uint8_t());
    rf_setup = ReadReg(Register::RF_SETUP);
    // printf("[DEBUG] [INIT] RF_SETUP: 0x%02X\n", rf_setup.to_uint8_t());

    // set the address of RX_pipeline 0 (this is us)
    WriteReg(Register::RX_ADDR_P0, device_address);
    
    // printf("[DEBUG] [INIT] Pipeline RX 0 Address: 0x%02X\n", ReadReg(Register::RX_ADDR_P0));

    // start up in recv mode
    CONFIG config = {0};
    config = ReadReg(Register::CONFIG);
    config.PRIM_RX = 1;
    WriteReg(Register::CONFIG, config.to_uint8_t());
    config = ReadReg(Register::CONFIG);

    // enable auto ack
    EN_AA en_aa = {0};
    en_aa = ReadReg(Register::EN_AA);
    en_aa.ENAA_P0 = 1;
    en_aa.ENAA_P1 = 1;
    en_aa.ENAA_P2 = 1;
    en_aa.ENAA_P3 = 1;
    en_aa.ENAA_P4 = 1;
    en_aa.ENAA_P5 = 1;
    WriteReg(Register::CONFIG, en_aa.to_uint8_t());
    en_aa = ReadReg(Register::EN_AA);



    FEATURE feature = {0};
    feature = ReadReg(Register::FEATURE);
    feature.EN_ACK_PAY = 1;
    WriteReg(Register::FEATURE, feature.to_uint8_t());
    feature = ReadReg(Register::FEATURE);

    SetStandbyMode();

}


bool NRF24::ChipAvaliable(){
    ReadReg(Register::CONFIG);
    return (bool) this->status.PWR_UP;
}
void NRF24::WriteReg(Register reg, uint8_t value){
    WriteReg((uint8_t) reg, value);
}

void NRF24::WriteReg(uint8_t reg, uint8_t value){
    this->tx_reg[0] = (uint8_t) Commands::W_REGISTER | reg;
    this->tx_reg[1] = value;
    this->tx_reg[2] = (uint8_t) Commands::NOP;
    this->tx_reg[3] = (uint8_t) Commands::NOP;
    this->rx_reg[0] = 0x00; // make sure its a clear register
    this->rx_reg[1] = 0x00; // make sure its a clear register

    start_transaction();
    spi_write_read_blocking(this->spi, this->tx_reg, this->rx_reg, 2);
    stop_transaction();

    
    this->status = this->rx_reg[0];
    // printf("[DEBUG] [RegWrite] reg: %02X, value: %02X\n",reg, value);
    // printf("[DEBUG] [RegWrite] reg: %02X, tx_reg[0]: %02X\n",reg, this->tx_reg[0]);
    // printf("[DEBUG] [RegWrite] reg: %02X, rx_reg[1]: %02X\n",reg, this->rx_reg[1]);
    
}

uint8_t NRF24::ReadReg(Register reg){
    return ReadReg((uint8_t) reg);
}

uint8_t NRF24::ReadReg(uint8_t reg){
    this->tx_reg[0] = (uint8_t) Commands::R_REGISTER | reg;
    this->tx_reg[1] = (uint8_t) Commands::NOP;
    this->rx_reg[0] = 0x00; // make sure its a clear register
    this->rx_reg[1] = 0x00; // make sure its a clear register


    start_transaction();
    spi_write_read_blocking(this->spi, this->tx_reg, this->rx_reg, 2);
    stop_transaction();

    
    this->status = this->rx_reg[0];
    // printf("[DEBUG] [RegRead] reg: %02X, value[0]: %02X\n",reg, this->rx_reg[0]);
    // printf("[DEBUG] [RegRead] reg: %02X, value[1]: %02X\n",reg, this->rx_reg[1]);
    return rx_reg[1];
    
}

void NRF24::GetState(){
    CONFIG config = {0};
    config = ReadReg(Register::CONFIG);
    uint8_t state = 0;
    if (config.PWR_UP & config.PRIM_RX){
        // RX Mode
        printf("RX Mode");
    }else if (config.PWR_UP & ~config.PRIM_RX)
    {
        printf("Standby-11");
    }
    
}

void NRF24::SetPowerDownMode(){
    CONFIG config = {0};
    config = ReadReg(Register::CONFIG);
    config.PWR_UP = 0;
    WriteReg(Register::CONFIG, config.to_uint8_t());
}

void NRF24::SetStandbyMode(){
    disable();
    CONFIG config = {0};
    config = ReadReg(Register::CONFIG);
    config.PWR_UP = 1;
    WriteReg(Register::CONFIG, config.to_uint8_t());
}

void NRF24::SetRXMode(){
    enable();
    CONFIG config = {0};
    config = ReadReg(Register::CONFIG);
    config.PRIM_RX = 1;
    config.PWR_UP = 1;
    WriteReg(Register::CONFIG, config.to_uint8_t());
}

void NRF24::SetTXMode(){
    enable();
    CONFIG config = {0};
    config = ReadReg(Register::CONFIG);
    config.PRIM_RX = 0;
    config.PWR_UP = 1;
    WriteReg(Register::CONFIG, config.to_uint8_t());
}