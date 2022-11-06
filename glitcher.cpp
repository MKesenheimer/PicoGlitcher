/*
 * glitcher for Raspberry Pi Pico
 *
 * @version     1.0.0
 * @author      Matthias Kesenheimer
 * @copyright   2021
 * @licence     GPL
 *
 */

/* 
 * Example settings:
 * 
 * ./controller.py /dev/tty.usbmodem0000000000001 --timeout 0.01  -p 10 15 1 -d 20 30 1
 * Generates a glitch after ~1us, with a length of ~0.5us.
 * 
 * Osciloscope:
 * Time/Div: 1us
 * External trigger enabled, rising edge, DC
 * 
*/

#include <stdio.h>
#include <iostream>
#include "tusb.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
extern "C" {
#include "pdnd/pdnd.h"
#include "pdnd/pdnd_display.h"
#include "pdnd/pio/pio_spi.h"
}
const uint SI_PIN = 3;
const uint SO_PIN = 4;
const uint CLK_PIN = 2;

const uint OUT_TARGET_POWER = 0;
const uint IN_NRF_VDD = 0;
const uint32_t TARGET_POWER = 18;

// This pin shows, if a glitching attack is active.
// Not necessary, but could probably be helpful somehow 
const uint PDND_GLITCH_ENABLE = 22;

const uint8_t CMD_DELAY = 0x64;
const uint8_t CMD_PULSE = 0x70;
const uint8_t CMD_GLITCH = 0x67;
const uint8_t CMD_HELLO = 0x68;
const uint8_t CMD_CHECK = 0x63;

static uint32_t delay, pulse;
static bool active;

void initialize_board() {
  gpio_init(TARGET_POWER);
  gpio_put(TARGET_POWER, 0);
  gpio_set_dir(TARGET_POWER, GPIO_OUT);
  gpio_init(PDND_GLITCH);
  gpio_put(PDND_GLITCH, 0);
  gpio_set_dir(PDND_GLITCH, GPIO_OUT);
  gpio_init(PDND_GLITCH_ENABLE);
  gpio_put(PDND_GLITCH_ENABLE, 0);
  gpio_set_dir(PDND_GLITCH_ENABLE, GPIO_OUT);
}

static inline void power_cycle_target() {
  gpio_put(TARGET_POWER, 0);
  sleep_ms(50);
  gpio_put(TARGET_POWER, 1);
}

void dv(uint32_t delay, uint32_t pulse) {
  cls(false);
  pprintf("Glitcher\nD: %lu\nP: %lu", delay, pulse);
}

void glitch(uint32_t delay, uint32_t pulse) {
  //power_cycle_target();
  gpio_put(PDND_GLITCH_ENABLE, 1);
  for (uint32_t i = 0; i < delay; ++i) {
    __asm__("NOP");
  }
  gpio_put(PDND_GLITCH, 1);
  for (uint32_t i = 0; i < pulse; ++i) {
    __asm__("NOP");
  }
  gpio_put(PDND_GLITCH_ENABLE, 0);
  gpio_put(PDND_GLITCH, 0);
}

int main() {
  // init
  stdio_init_all();
  stdio_set_translate_crlf(&stdio_usb, false);
  
  // Wait for client to connect
  //while (!tud_cdc_connected()) { sleep_ms(100);  }
  //printf("tud_cdc_connected()\n");

  pdnd_initialize();
  pdnd_enable_buffers(0);
  pdnd_display_initialize();
  pdnd_enable_buffers(1);
  cls(false);
  pprintf("Glitcher");

  // Sets up trigger & glitch output
  initialize_board();

  active = false;
  while(1) {
    uint8_t cmd = std::cin.get();
    switch (cmd) {
      case CMD_HELLO:
        cls(false);
        pprintf("Glitcher\nClient connected.");
        break;
      case CMD_DELAY:
        gpio_put(PDND_GLITCH_ENABLE, 0);
        std::cin >> delay;
        dv(delay, pulse);
        break;
      case CMD_PULSE:
        std::cin >> pulse;
        dv(delay, pulse);
        break;
      case CMD_GLITCH:
        power_cycle_target();
        // wait for start-up
        while(!gpio_get(1 + IN_NRF_VDD));
        glitch(delay, pulse);
        break;
      case CMD_CHECK:
        std::cout << "pio -> d: " << delay << ", p: " << pulse << std::endl;
        break;
      default:
        break;
    }
  }

  return 0;
}