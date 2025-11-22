# TODO
1. Make the debug setup
    1. Make 3D cad file to 3D print
    2. ~~Solder headers onto board~~
    3. Glue stuff down so it cant move
    4. include lights in this
2. ~~get blinky working with debug setup~~
3. ~~get basic comms working for debug setup~~
    1. ~~UART to send basic commands~~
    2. ~~Multicore?~~
4. ~~create command parser~~
5. ~~set and get basic commands~~
6. ~~set and get frames~~
7. ~~PIO outputs for frame~~
8. set and get memory (need extra dev board)
9. wireless comms




# Links
[Dev Board Specific](https://docs.wiznet.io/Product/Chip/Ethernet/W5500/w5500-evb-pico)
[Dev Board Simple](https://wiznet.io/products/evaluation-boards/w5500-evb-pico)
[RP2040 Datasheet](https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf)

[Debug Probe Setup](https://www.raspberrypi.com/documentation/microcontrollers/debug-probe.html)
[Debug probe WIP](https://www.raspberrypi.com/documentation/microcontrollers/debug-probe.html#:~:text=If%20you%20have,Debug%20Probe%2C%20respectively.)

[Wireless Chip - Amazon](https://www.amazon.com/Makerfire-Arduino-NRF24L01-Wireless-Transceiver/dp/B00O9O868G)
[Wireless Chip Datasheet - Sparkfun](https://cdn.sparkfun.com/assets/3/d/8/5/1/nRF24L01P_Product_Specification_1_0.pdf)

[Offboard Memory - Adafruit](https://www.adafruit.com/product/5634)

[RP2040 Multicore - Video](https://www.youtube.com/watch?v=aIFElaK14V4)


## Dev Setup
### NRF24 Wiring
IRQ -> ORANGE
MISO -> YELLOW
MOSI -> GREEN
SCK -> BLUE
CSN -> PURPLE
CE -> GREY
VCC -> WHITE
GND -> BLACK

IRQ -> Pico GPIO 7 (pin 10)
MOSI -> Pico GPIO 3 (pin 5)
MISO -> Pico GPIO 4 (pin 6)
SCK -> Pico GPIO 2 (pin 4)
SCN -> Pico GPIO 5 (pin 7)
CE -> Pico GPIO 6 (pin 9)
VCC -> Pico 3v3 (pin 36)
GND -> Pico GND (pin 8)

Blue -> 4
Green -> 5
Yellow -> 6
Purple -> 7
Black -> 8
Grey -> 9
Orange -> 10
White -> 36