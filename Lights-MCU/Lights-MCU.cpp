#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/mutex.h"
#include "pico/time.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/interp.h"
#include "hardware/timer.h"
#include "hardware/watchdog.h"
#include "hardware/clocks.h"
#include "hardware/uart.h"
#include <string>

#include "include/parsing.h"

#include "blink.pio.h"


// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9

// Data will be copied from src to dst
const char src[] = "Hello, world! (from DMA)";
char dst[count_of(src)];

static mutex_t uart_mutex;


void blink_pin_forever(PIO pio, uint sm, uint offset, uint pin, uint freq) {
    blink_program_init(pio, sm, offset, pin);
    pio_sm_set_enabled(pio, sm, true);

    printf("Blinking pin %d at %d Hz\n", pin, freq);

    // PIO counter program takes 3 more cycles in total than we pass as
    // input (wait for n + 1; mov; jmp)
    pio->txf[sm] = (125000000 / (2 * freq)) - 3;
}


int64_t alarm_callback(alarm_id_t id, void *user_data) {
    // Put your timeout handler code in here
    return 0;
}



// UART defines
// By default the stdout UART is `uart0`, so we will use the second one
#define UART_ID uart0
#define BAUD_RATE 115200

#define UART_TX_PIN 0
#define UART_RX_PIN 1


#define MAX_PAYLOAD_LEN 8

volatile uint8_t recv_char;
extern volatile uint8_t uart_buffer[];
extern volatile uint8_t payload_len;
extern volatile ParseState uart_parsing_state;

volatile uint32_t current_time;
extern volatile uint32_t last_time;

volatile Config light_config = {0,0,0,0};

// the amount of time that has passed that the message should be considered invalid
constexpr uint32_t uart_invalid_timeout_us = 10000;


void core1_entry(void){
    mutex_enter_blocking(&uart_mutex);
    printf("Core 1 Started\n");
    mutex_exit(&uart_mutex);

    // char recv_char = ' ';
    std::string result;
    char sentinel = '\n';
    char uart_buff[255];
    char temp_hex[6];

   
    while (true){

        current_time = time_us_32();

        if (((current_time - last_time) > uart_invalid_timeout_us) and (uart_parsing_state != ParseState::WAIT_START)){
            uart_parsing_state = ParseState::WAIT_START;
            mutex_enter_blocking(&uart_mutex);
            uart_puts(UART_ID, "Message Invalid, State Timeout");
            mutex_exit(&uart_mutex);
        }
        
        if (uart_parsing_state == ParseState::WAIT_FOR_PROCESSING){
            sprintf(uart_buff, "VER: %02x, CMD: %02x, LEN: %02x, PLD:[%02x,%02x,%02x,%02x,%02x]\n", 
                    uart_buffer[0], // ver
                    uart_buffer[1], // cmd
                    uart_buffer[2], // len
                    uart_buffer[3],uart_buffer[4],uart_buffer[5],uart_buffer[6],uart_buffer[7]
                );
            mutex_enter_blocking(&uart_mutex);
            uart_puts(UART_ID, uart_buff);
            mutex_exit(&uart_mutex);
            Result<uint16_t> result = parse_payload(uart_buffer, payload_len);
            uart_parsing_state = ParseState::WAIT_START; // finished processing, put it back to waiting for the next command

            // format the results for sending back
            sprintf(uart_buff, "Value: %04X, Error: %02X \n", result.value, result.error);

            mutex_enter_blocking(&uart_mutex);
            uart_puts(UART_ID, uart_buff);
            mutex_exit(&uart_mutex);

        }
        
        sleep_ms(2);
    }
}





// RX interrupt handler
void on_uart_rx() {
    while (uart_is_readable(UART_ID)) {
        recv_char = uart_getc(UART_ID);
        process_byte(recv_char);
    }
}

void setup_uart(){
    // Set up our UART
    uart_init(UART_ID, BAUD_RATE);
    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_hw_flow(UART_ID, false, false);

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
}

void setup_SPI(){
     // // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
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
    
    
    // setup_SPI();
    
   
    
    multicore_launch_core1(core1_entry);
    sleep_ms(500);

    // uint32_t check = multicore_fifo_pop_blocking();
    // if (check != MULTI_CORE_FLAG_VALUE){
    //     printf("There was an error starting core 1");
    // }
    // else{
    //     multicore_fifo_push_blocking(MULTI_CORE_FLAG_VALUE);
    //     printf("Core 1 was started correctly (this is from core 0)");
    // }

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

    // Get a free channel, panic() if there are none
    // int chan = dma_claim_unused_channel(true);
    
    // 8 bit transfers. Both read and write address increment after each
    // transfer (each pointing to a location in src or dst respectively).
    // No DREQ is selected, so the DMA transfers as fast as it can.
    
    // dma_channel_config c = dma_channel_get_default_config(chan);
    // channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    // channel_config_set_read_increment(&c, true);
    // channel_config_set_write_increment(&c, true);
    
    // dma_channel_configure(
    //     chan,          // Channel to be configured
    //     &c,            // The configuration we just created
    //     dst,           // The initial write address
    //     src,           // The initial read address
    //     count_of(src), // Number of transfers; in this case each is 1 byte.
    //     true           // Start immediately.
    // );
    
    // // We could choose to go and do something else whilst the DMA is doing its
    // // thing. In this case the processor has nothing else to do, so we just
    // // wait for the DMA to finish.
    // dma_channel_wait_for_finish_blocking(chan);
    
    // // The DMA has now copied our text from the transmit buffer (src) to the
    // // receive buffer (dst), so we can print it out from there.
    // puts(dst);

    // PIO Blinking example
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &blink_program);
    sprintf(uart_buff, "Loaded program at %d\n", offset);
    mutex_enter_blocking(&uart_mutex);
    uart_puts(UART_ID, uart_buff);
    mutex_exit(&uart_mutex);
    
    #ifdef PICO_DEFAULT_LED_PIN
    blink_pin_forever(pio, 0, offset, PICO_DEFAULT_LED_PIN, 3);
    #else
    blink_pin_forever(pio, 0, offset, 6, 3);
    #endif
    // For more pio examples see https://github.com/raspberrypi/pico-examples/tree/master/pio

    // Interpolator example code
    interp_config cfg = interp_default_config();
    // Now use the various interpolator library functions for your use case
    // e.g. interp_config_clamp(&cfg, true);
    //      interp_config_shift(&cfg, 2);
    // Then set the config 
    interp_set_config(interp0, 0, &cfg);
    // For examples of interpolator use see https://github.com/raspberrypi/pico-examples/tree/master/interp

    // Timer example code - This example fires off the callback after 2000ms
    // add_alarm_in_ms(2000, alarm_callback, NULL, false);
    // For more examples of timer use see https://github.com/raspberrypi/pico-examples/tree/master/timer

    // Watchdog example code
    // if (watchdog_caused_reboot()) {
    //     printf("Rebooted by Watchdog!\n");
    //     // Whatever action you may take if a watchdog caused a reboot
    // }
    
    // Enable the watchdog, requiring the watchdog to be updated every 100ms or the chip will reboot
    // second arg is pause on debug which means the watchdog will pause when stepping through code
    // watchdog_enable(100, 1);
    
    // You need to call this function at least more often than the 100ms in the enable call to prevent a reboot
    // watchdog_update();
    mutex_enter_blocking(&uart_mutex);
    sprintf(uart_buff, "System Clock Frequency is %d Hz\n", clock_get_hz(clk_sys));
    uart_puts(UART_ID, uart_buff);
    sprintf(uart_buff, "USB Clock Frequency is %d Hz\n", clock_get_hz(clk_usb));
    uart_puts(UART_ID, uart_buff);
    mutex_exit(&uart_mutex);
    // For more examples of clocks use see https://github.com/raspberrypi/pico-examples/tree/master/clocks

   
    while (true) {
        mutex_enter_blocking(&uart_mutex);
        uart_puts(UART_ID, "Current State:");
        printf("%02X\n", uart_parsing_state);
        mutex_exit(&uart_mutex);
        sleep_ms(1000);
    }
}
