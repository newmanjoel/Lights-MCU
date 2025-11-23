#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/mutex.h"
#include "pico/time.h"
#include "hardware/irq.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/interp.h"
#include "hardware/timer.h"
#include "hardware/watchdog.h"
#include "hardware/clocks.h"
#include "hardware/uart.h"
// #include "pico/binary_info.h"
#include <string>

#include "include/parsing.h"

#include "blink.pio.h"
#include "WS2811.pio.h"
#include "constants.h"
#include "light_hal.h"
#include "nRF24L01P.h"
#include "files.h"
#include "ArduinoJson-v7.4.2.h"
#include <cstring>



static mutex_t uart_mutex;

volatile uint8_t recv_char;


volatile uint32_t current_time;
volatile uint32_t time_last_byte_recvd;

volatile Animation_Config light_config = {250,100,2,0,0,0,1,0,1,0};

uint32_t next_frame[max_led_len] = {0};
uint32_t current_frame[max_led_len] = {0};
volatile uint8_t current_file = 0;
volatile uint32_t playback_location = 0;

volatile File files[10];
volatile uint32_t data[max_data_len] = {0};


volatile uint32_t led_frame[max_frame_len][max_led_len] = {0};
volatile uint32_t working_frame_index = 0;
volatile int dma_chan;

char uart_buff[250];
char status_buff[500];
JsonDocument status;
uint16_t debug_working_index = 0;

NRF_HAL spi_hal;
NRF24 wireless;


void output_byte(uint8_t b) {
    // uart_putc_raw(uart0, b);  // hardware UART Only
    putchar_raw(b);           // Both UART AND USB
}

void uart_out(char* buffer, int buffer_len){
    mutex_enter_blocking(&uart_mutex);
    for(int i=0; i< buffer_len; i++){
        if (buffer[i] == 0){ // NULL ending
            mutex_exit(&uart_mutex);
            return;
        }
        output_byte(buffer[i]);
    }
    // output_byte(0); // NULL ending
    mutex_exit(&uart_mutex);
}




void core1_entry(void){
    mutex_enter_blocking(&uart_mutex);
    printf("Core 1 Started\n");
    mutex_exit(&uart_mutex);
    JsonDocument result;

    

    while (true){
        
        current_time = time_us_32();

        if (((current_time - time_last_byte_recvd) > uart_invalid_timeout_us)
        and (current_time > time_last_byte_recvd)
        and (Parsing::uart_parsing_state != ParseState::WAIT_START)){
            // mutex_enter_blocking(&uart_mutex);
            auto time_diff = current_time - time_last_byte_recvd;
            char temp_buff[50];
            sprintf(temp_buff, "State_timeout. Time Diff: %d us\n", time_diff);
            mutex_enter_blocking(&uart_mutex);
            uart_puts(uart0, temp_buff);
            mutex_exit(&uart_mutex);
            // printf("State was: %02x and WAIT_START is %02x\n", Parsing::uart_parsing_state, ParseState::WAIT_START);
            // mutex_exit(&uart_mutex);
            Parsing::uart_parsing_state = ParseState::WAIT_START;
        }
        
        if (Parsing::uart_parsing_state == ParseState::WAIT_FOR_PROCESSING){
            if(light_config.debug_cmd){
                sprintf(uart_buff, "VER: %02x, CMD: %02x, LEN: %02x, PLD: [", 
                        Parsing::uart_buffer[0], // ver
                        Parsing::uart_buffer[1], // cmd
                        Parsing::uart_buffer[2] // len
                    );

                for (int i = 0;i<Parsing::payload_len; i++){
                    sprintf(uart_buff, "%s %2X", 
                        uart_buff,
                        Parsing::uart_buffer[3+i]
                    );
                }
                sprintf(uart_buff, "%s] \n", 
                    uart_buff
                );
                mutex_enter_blocking(&uart_mutex);
                uart_puts(UART_ID, uart_buff);
                mutex_exit(&uart_mutex);
            }
            
            uint32_t timing = time_us_32();
            result.clear();
            status["Timing"]["result_clear"] = time_us_32()-timing;
            // Total processing time for a FILE::GET is about 155 us
            timing = time_us_32();
            parse_payload(result, Parsing::uart_buffer, Parsing::payload_len);
            
            status["Timing"]["e2e"] = time_us_32()-time_last_byte_recvd;
            status["Timing"]["parse"] = time_us_32()-timing;
            
            // format the results for sending back
            // sprintf(uart_buff, "Value: %08X, Error: %02X \n", result.value, result.error);
            
            auto length = serializeJson(result, uart_buff);
            uart_buff[length] = '\n';

            uart_out(uart_buff, 250);
            clear_uart_buffer(uart_buff, 250);
            // uart_out(['\n'], 255);
            
            // mutex_enter_blocking(&uart_mutex);
            // uart_puts(UART_ID, uart_buff);
            // uart_putc(UART_ID, '\n');
            // mutex_exit(&uart_mutex);

            // mutex_enter_blocking(&uart_mutex);
            
            // mutex_exit(&uart_mutex);

            
            Parsing::uart_parsing_state = ParseState::WAIT_START; // finished processing, put it back to waiting for the next command

        }
        
    }
}


// RX interrupt handler
void on_uart_rx() {
    while (uart_is_readable(UART_ID)) {
        recv_char = uart_getc(UART_ID);
        process_byte(recv_char);
        time_last_byte_recvd = time_us_32();
    }
}

void setup_uart(){
    // Set up our UART
    uart_init(UART_ID, BAUD_RATE);
    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    gpio_set_slew_rate(UART_TX_PIN, GPIO_SLEW_RATE_FAST);
    gpio_set_drive_strength(UART_RX_PIN, GPIO_DRIVE_STRENGTH_12MA);

    uart_set_hw_flow(UART_ID, false, false);

    /* Disabling the interrupts in favor of polling for now, to get the USB working. 
    // Set up a RX interrupt
    // We need to set up the handler first
    // Select correct interrupt for the UART we are using
    int UART_IRQ = UART_ID == UART_ID ? UART0_IRQ : UART1_IRQ;

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(UART_ID, true, false);
    
    // Use some the various UART functions to send out data
    // In a default system, printf will also output via the default UART
    // For more examples of UART use see https://github.com/raspberrypi/pico-examples/tree/master/uart
    */
}

void setup_SPI(){
    //  // // SPI initialisation. This example will use SPI at 1MHz.
    // spi_init(SPI_PORT, 1000*1000);
    // gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    // gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    // gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    // gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    spi_hal = {
        2, // sck
        3, // mosi
        4, // miso
        6, // ce
        5, // chip select
        7, // irq
    };
    wireless.init(spi_hal, spi0, 0x55);
    
}

void blink_pin_forever(PIO pio, uint sm, uint offset, uint pin, uint freq) {
    blink_program_init(pio, sm, offset, pin);
    pio_sm_set_enabled(pio, sm, true);

    printf("Blinking pin %d at %d Hz\n", pin, freq);

    // PIO counter program takes 3 more cycles in total than we pass as
    // input (wait for n + 1; mov; jmp)
    pio->txf[sm] = (125000000 / (2 * freq)) - 3;
}

Pio_SM_info setup_Blinky_Pio(){
    // PIO Blinking example
    PIO pio = pio0;
    uint sm = pio_claim_unused_sm(pio, true);
    uint offset = pio_add_program(pio, &blink_program);
    blink_pin_forever(pio, sm, offset, PICO_DEFAULT_LED_PIN, 3);
    // For more pio examples see https://github.com/raspberrypi/pico-examples/tree/master/pio
    return {pio, sm, offset};
}

Pio_SM_info setup_WS2811_Pio(){
    // PIO Blinking example
    constexpr uint data_output_pin = 13;
    gpio_set_dir(data_output_pin, true);
    // gpio_set_function(data_output_pin, GPIO_FUNC_PIO1);
    PIO pio = pio1;
    uint offset = pio_add_program(pio, &ws2811_program);
    uint sm = pio_claim_unused_sm(pio, true);
    pio_sm_config config = ws2811_program_init(pio, sm, offset, data_output_pin);

    pio_sm_set_enabled(pio, sm, true);

    // For more pio examples see https://github.com/raspberrypi/pico-examples/tree/master/pio
    return {pio, sm, offset};
}

uint32_t rgb_to_int(uint8_t red, uint8_t green, uint8_t blue){
    // format is GRB
    uint32_t rgb = (red << 16)|(green<<8) | blue;
    return rgb;
}

bool system_status_report(__unused repeating_timer_t *rt){
    if(light_config.status_report){
        // NRF24_Registers::RX_PWR_D recv_power_dector = {0};
        

        
        
        // recv_power_dector = wireless.ReadReg(NRF24_Registers::Register::RX_PWR_D);

        status["Status"]["playback_location"] = playback_location;
        status["Status"]["Current_File"] = current_file;
        status["Status"]["current_file_start"] = files[current_file].start;
        status["Status"]["current_file_end"] = files[current_file].end;

        status["Config"]["fps"] =light_config.fps_ms;
        status["Config"]["running"] =light_config.running;
        status["Config"]["led_count"] =light_config.led_count;
        status["Config"]["frame_count"] =light_config.frame_count;
        status["Config"]["debug_cmd"] =light_config.debug_cmd;

        // status["Wireless"]["Power"] =wireless.status.PWR_UP;
        // status["Wireless"]["RX Mode"] =wireless.status.PRIM_RX;
        // status["Wireless"]["Revc Pwr Dector"] =recv_power_dector.RPD;

        // clear_uart_buffer(buffer, 500);
        clear_uart_buffer(status_buff, 500);
        auto length = serializeJsonPretty(status, status_buff);
        status_buff[length] = '\n';

        // uart_out(status_buff, 500);

        mutex_enter_blocking(&uart_mutex);
        uart_puts(UART_ID, status_buff); // only put on hardware UART
        mutex_exit(&uart_mutex);
        status.remove("Debug");
        debug_working_index = 0;
        status.remove("Timing");
        // status.clear();
    }
    return true;
}


bool push_data_to_lights_callback(__unused repeating_timer_t *rt){
    working_frame_index = (working_frame_index+1) % light_config.frame_count;
    
    rt->delay_us = ((int64_t) light_config.fps_ms)*-1000;

    // for some reason these didn't work?
    // dma_channel_set_transfer_count(dma_chan, light_config.led_count, false);
    // dma_channel_set_write_addr(dma_chan, &led_frame[working_frame_index][0], true);


    if (light_config.running)
        memcpy(current_frame, next_frame, sizeof(next_frame));
        // for (int i=0; i<light_config.led_count; i++){
        //     current_frame[i] = next_frame[i];
        // }
        dma_channel_transfer_from_buffer_now(dma_chan,&current_frame, (uint32_t) light_config.led_count);
        // set up the next frame for the next loop. The DMA is happening in the background so we dont have to worry about timeing
        volatile int i = 0;
        volatile int j = 0;
        volatile int temp_index = 0;
        volatile uint8_t count = 0;

        // data is a continous section of memory for all of the light colors in RLE form
        // current file is just the index of the current file that we are playing back
        // we need to read data from the last place we were and keep reading until we have hit the config.led_count ammount
        // playback location is the last position that we read +1, start from this location next time
        // TODO: right now i is not indexing correctly
        
        if (current_file != 0){
            current_file = current_file %10;
        }
        count = data[playback_location] & 0xFF;
        if (count == 0){
            count =1;
        }
        for (i=0; i<light_config.led_count; i++){
            --count;
            next_frame[i] = data[playback_location];
            if (count == 0){
                playback_location += 1;
                count = data[playback_location] &0xFF;
                if (count == 0){
                    count =1;
                }

            }
            
            
            if (playback_location > files[current_file].end){
                if (files[current_file].action == EndAction::REPEAT){
                    if (i != (light_config.led_count - 1)){
                        printf("Something has gone wrong");
                    }
                    playback_location = (uint32_t) files[current_file].start;
                }
                else if (files[current_file].action == EndAction::RUN_FILE){
                    // TODO: figure out a way to set this kind of info
                }
                
            }
        }
        // printf("Next ", playback_location);
        
       
    return true; // keep repeating
}

void default_file_0(){
    // setup a basic static color for file 0
    uint8_t temp = 0x05;
    data[0] =  temp+ (rgb_to_int(168, 136, 20) << 8);
    temp = 95;
    data[1] =  temp+ (rgb_to_int(100, 100, 100) << 8);
    files[0].start = 0;
    files[0].end = 1;
    files[0].action = EndAction::REPEAT;
    playback_location = files[0].start;

}

void poll_uarts() {
    // --- 1. Read from hardware UART ---
    while (uart_is_readable(uart0)) {
        uint8_t b = uart_getc(uart0);
        time_last_byte_recvd = time_us_32();
        process_byte(b);
    }

    // --- 2. Read from USB CDC (stdio_usb) ---
    int c;
    while ((c = getchar_timeout_us(100)) != PICO_ERROR_TIMEOUT) {
        time_last_byte_recvd = time_us_32();
        // char temp_buff[50];
        // sprintf(temp_buff, "Before: C:%d S:%d\n", c, (uint8_t) Parsing::uart_parsing_state);
        // mutex_enter_blocking(&uart_mutex);
        // uart_puts(uart0, temp_buff);
        // mutex_exit(&uart_mutex);
        process_byte((uint8_t)c);
        // sprintf(temp_buff, "After: C:%d S:%d\n", c, (uint8_t) Parsing::uart_parsing_state);
        // mutex_enter_blocking(&uart_mutex);
        // uart_puts(uart0, temp_buff);
        // mutex_exit(&uart_mutex);
        
        
    }
}



int main()
{
    stdio_init_all();

    mutex_init(&uart_mutex);
    sleep_ms(1); // If this is not here then the whole system hangs forever.
    char uart_buff[50];


    setup_uart();
    
    // Send out a string, with CR/LF conversions
    mutex_enter_blocking(&uart_mutex);
    uart_puts(UART_ID, " Hello, UART!\n");
    mutex_exit(&uart_mutex);
    
    
    setup_SPI();
    NRF24_Registers::CONFIG reg1;
    NRF24_Registers::CONFIG reg2;
    wireless.ChipAvaliable();
    reg1 = wireless.status;
    // wireless.WriteReg(0x00, 0x01);

    

    // Full register dump of everything
    wireless.ReadReg((uint8_t)NRF24_Registers::Register::CONFIG);
    wireless.ReadReg((uint8_t)NRF24_Registers::Register::EN_AA);
    wireless.ReadReg((uint8_t)NRF24_Registers::Register::EN_RXADDR);
    wireless.ReadReg((uint8_t)NRF24_Registers::Register::SETUP_AW);
    wireless.ReadReg((uint8_t)NRF24_Registers::Register::SETUP_RETR);
    wireless.ReadReg((uint8_t)NRF24_Registers::Register::RF_CHANNEL);
    wireless.ReadReg((uint8_t)NRF24_Registers::Register::RF_SETUP);
    wireless.ReadReg((uint8_t)NRF24_Registers::Register::STATUS);
    wireless.ReadReg((uint8_t)NRF24_Registers::Register::OBSERVE_TX);
    wireless.ReadReg((uint8_t)NRF24_Registers::Register::RX_PWR_D);
    wireless.ReadReg((uint8_t)NRF24_Registers::Register::RX_ADDR_P0);
    wireless.ReadReg((uint8_t)NRF24_Registers::Register::RX_ADDR_P1);
    wireless.ReadReg((uint8_t)NRF24_Registers::Register::RX_ADDR_P2);
    wireless.ReadReg((uint8_t)NRF24_Registers::Register::RX_ADDR_P3);
    wireless.ReadReg((uint8_t)NRF24_Registers::Register::RX_ADDR_P4);
    wireless.ReadReg((uint8_t)NRF24_Registers::Register::RX_ADDR_P5);
    wireless.ReadReg((uint8_t)NRF24_Registers::Register::TX_ADDR);
    wireless.ReadReg((uint8_t)NRF24_Registers::Register::RX_PL_P0);
    wireless.ReadReg((uint8_t)NRF24_Registers::Register::RX_PL_P1);
    wireless.ReadReg((uint8_t)NRF24_Registers::Register::RX_PL_P2);
    wireless.ReadReg((uint8_t)NRF24_Registers::Register::RX_PL_P3);
    wireless.ReadReg((uint8_t)NRF24_Registers::Register::RX_PL_P4);
    wireless.ReadReg((uint8_t)NRF24_Registers::Register::RX_PL_P5);
    wireless.ReadReg((uint8_t)NRF24_Registers::Register::FIFO_STATUS);
    wireless.ReadReg((uint8_t)NRF24_Registers::Register::DYNPD);
    wireless.ReadReg((uint8_t)NRF24_Registers::Register::FEATURE);

    NRF24_Registers::STATUS wireless_status ={0};
    wireless_status.MAX_RT = 1;
    printf("Just Max RT: 0x%02X\n", wireless_status.to_uint8_t());



    // sleep_ms(200);

    mutex_enter_blocking(&uart_mutex);
    printf("NRF24 config:\n\treserved:%b\n\tMASK_RX_DR:%b\n\tMASK_TX_DS:%b\n\tMASK_MAX_RT:%b\n\tEN_CRC:%b\n\tCRCO:%b\n\tPWR_UP:%b\n\tPRIM_RX:%b\n", 
        reg2.Reserved,
        reg2.MASK_RX_DR,
        reg2.MASK_TX_DS,
        reg2.MASK_MAX_RT,
        reg2.EN_CRC,
        reg2.CRCO,
        reg2.PWR_UP,
        reg2.PRIM_RX);
    mutex_exit(&uart_mutex);


   
    
    multicore_launch_core1(core1_entry);
    sleep_ms(500);

    

    // // Chip select is active-low, so we'll initialise it to a driven-high state
    // gpio_set_dir(PIN_CS, GPIO_OUT);
    // gpio_put(PIN_CS, 1);
    // // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    // // I2C Initialisation. Using it at 400Khz.
    // i2c_init(I2C_PORT, 400*1000);
    
    // gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    // gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    // gpio_pull_up(I2C_SDA);
    // gpio_pull_up(I2C_SCL);
    // // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

    
    Pio_SM_info blinky_pio = setup_Blinky_Pio();
    sprintf(uart_buff, "Blinky sm: %d, offset: %d \n", blinky_pio.sm, blinky_pio.offset);
    mutex_enter_blocking(&uart_mutex);
    uart_puts(UART_ID, uart_buff);
    mutex_exit(&uart_mutex);
    
    Pio_SM_info ws2811_pio = setup_WS2811_Pio();
    sprintf(uart_buff, "ws2811 sm: %d, offset: %d \n", ws2811_pio.sm, ws2811_pio.offset);
    mutex_enter_blocking(&uart_mutex);
    uart_puts(UART_ID, uart_buff);
    mutex_exit(&uart_mutex);
    
    // Get a free channel, panic() if there are none
    uint temp_dma_chan = dma_claim_unused_channel(true);
    
    dma_channel_config dma_config = dma_channel_get_default_config(temp_dma_chan);
    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_32);
    channel_config_set_dreq(&dma_config, pio_get_dreq(ws2811_pio.pio, ws2811_pio.sm, true));
    channel_config_set_read_increment(&dma_config, true);
    channel_config_set_write_increment(&dma_config, false);
    dma_chan = temp_dma_chan;

    dma_channel_configure(
                dma_chan,
                &dma_config,
                &ws2811_pio.pio->txf[ws2811_pio.sm],
                &current_frame,
                (uint32_t) light_config.led_count,
                false
            );

    
    default_file_0();

    repeating_timer_t timer;

    // negative timeout means exact delay (rather than delay between callbacks)
    if (!add_repeating_timer_us(-1000000/20, push_data_to_lights_callback, NULL, &timer)) {
        printf("Failed to add timer\n");
        return 1;
    }

    repeating_timer_t system_status_timer;

    // report back general status every 2 seconds
    if (!add_repeating_timer_us(2000000, system_status_report, NULL, &system_status_timer)) {
        printf("Failed to add timer\n");
        return 1;
    }
    light_config.debug_cmd = false;
    status.clear();
    light_config.status_report = true;

    mutex_enter_blocking(&uart_mutex);
    sprintf(uart_buff, "System Clock Frequency is %d Hz\n", clock_get_hz(clk_sys));
    uart_puts(UART_ID, uart_buff);
    sprintf(uart_buff, "USB Clock Frequency is %d Hz\n", clock_get_hz(clk_usb));
    uart_puts(UART_ID, uart_buff);
    mutex_exit(&uart_mutex);
    // For more examples of clocks use see https://github.com/raspberrypi/pico-examples/tree/master/clocks



    while (true) {
        // tight_loop_contents();
        poll_uarts();
    }
}
