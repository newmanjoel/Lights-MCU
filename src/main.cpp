#include <Arduino.h>
// #include <pico/stdlib.h>
#include <pico-sdk/boards/include/boards/pico.h>
#include <pico/multicore.h>
#include <hardware/irq.h>
#include "command_structs.h"
#include <cstdio>

#define STR_BUFFER_LEN 50
#define LED_MAX_LEN 1024
#define FRAME_MAX_LEN 1024
#define BUILTIN_LED 25

Command Working_Command;
Config Working_Config;
char Str_Buffer[STR_BUFFER_LEN];
uint16_t temp;
uint8_t command_temp_buff[6];
size_t command_bytes_read;




uint32_t frameA[LED_MAX_LEN];
uint32_t frameB[LED_MAX_LEN];

// put function declarations here:
int myFunction(int, int);
bool read_command();
void update_lights();
void send_buffer();
void reset_command(Command &);
bool config_set(uint8_t config_id, uint16_t config_value);
uint16_t config_get(uint8_t config_id);

void core1_interrupt_handler()
{
}
void core1_entry()
{
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

  reset_command(Working_Command);
  Serial.println("Setup Complete");
  Serial.setTimeout(100);
  pinMode(BUILTIN_LED, OUTPUT);
  // gpio_init(BUILTIN_LED);
  gpio_set_dir(BUILTIN_LED, true);
  gpio_put(BUILTIN_LED, 1);
  // digitalWrite(BUILTIN_LED, 1);
  // multicore_launch_core1(core1_entry); // start core 1 - must be called before configuring interrupts

  // tight_loop_contents()
}

void loop()
{
  // put your main code here, to run repeatedly:
  switch (Working_Command.id)
  {
  case 1:
    temp = (bool)config_set(Working_Command.param1, Working_Command.param2);
    reset_command(Working_Command);
    break;
  case 2:
    temp = config_get(Working_Command.param1);
    std::snprintf(Str_Buffer, STR_BUFFER_LEN, "%u %u", Working_Command.param1, temp);
    send_buffer();
    reset_command(Working_Command);
    break;

  default:
    // do nothing
    break;
  };
  gpio_put(BUILTIN_LED, 1);
  sleep_ms(400);
  // (void)read_command();
  gpio_put(BUILTIN_LED, 0);
  sleep_ms(400);
}

// put function definitions here:
int myFunction(int x, int y)
{
  return x + y;
}

bool read_command(){
  // command_temp_buff[6];
  memset(command_temp_buff, 0, 6*sizeof(command_temp_buff[0])); // clear the temp buffer before use
  command_bytes_read = Serial.readBytesUntil('\n', (char*) command_temp_buff, sizeof(command_temp_buff));

  // if (command_bytes_read > sizeof(command_temp_buff)){
  //   return false;
  // }

  Working_Command.id = command_temp_buff[0];
  Working_Command.param1 = command_temp_buff[1];
  Working_Command.param2 = command_temp_buff[2] | (command_temp_buff[3] << 8);
  Working_Command.param3 = command_temp_buff[4] | (command_temp_buff[5] << 8);

  return true;
}

void send_buffer()
{
  Serial.println(Str_Buffer);
}

// This is run on core 1.
void update_lights()
{
  while (true)
  {
    tight_loop_contents();
  };
}

void reset_command(Command &command)
{
  command.id = 0;
  command.param1 = 0;
  command.param2 = 0;
  command.param3 = 0;
}

bool config_set(uint8_t config_id, uint16_t config_value)
{
  switch (config_id)
  {
  case 0:
    Working_Config.fps = (uint8_t)config_value;
    break;
  case 1:
    Working_Config.frame_count = config_value;
    break;
  case 2:
    Working_Config.led_count = config_value;
    break;
  default:
    return false;
  }
  return true;
}

uint16_t config_get(uint8_t config_id)
{
  switch (config_id)
  {
  case 0:
    return Working_Config.fps;
  case 1:
    return Working_Config.frame_count;
  case 2:
    return Working_Config.led_count;
  }
  return 0xFFFF;
}