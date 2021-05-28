/*
 * glitcher for Raspberry Pi Pico
 *
 * @version     1.0.0
 * @author      Matthias Kesenheimer
 * @copyright   2021
 * @licence     GPL
 *
 */

#include <stdio.h>
#include <iostream>
#include "pico/stdlib.h"

const uint PDND_GLITCH = 22;
const uint PDND_GLITCH_ACTIVE = 26;

const char CMD_DELAY = 'd';
const char CMD_PULSE = 'p';
const char CMD_GLITCH = 'g';

// GPIO Pin to read the Power Voltage from
const int IN_NRF_VDD = 0;

// TODO
static inline void power_cycle_target() {
  while(!gpio_get(IN_NRF_VDD));
}

int main() {
  // init
  stdio_init_all();
  gpio_init(PDND_GLITCH);
  gpio_set_dir(PDND_GLITCH, GPIO_OUT);
  gpio_init(PDND_GLITCH_ACTIVE);
  gpio_set_dir(PDND_GLITCH_ACTIVE, GPIO_OUT);
  
  uint32_t delay, pulse;
  while(1) {
    uint8_t cmd = std::cin.get();
    switch (cmd) {
      case CMD_DELAY:
        std::cin >> delay;
        //std::cout << "setting delay to " << delay << std::endl;
        break;
      case CMD_PULSE:
        std::cin >> pulse;
        //std::cout << "setting pulse to " << pulse << std::endl;
        break;
      case CMD_GLITCH:
        std::cout << "delay = " << delay << ", pulse = " << pulse << std::endl;
        //power_cycle_target();
        gpio_put(PDND_GLITCH_ACTIVE, 1);
        for (uint32_t i = 0; i < delay; ++i) {
          __asm__("NOP");
        }
        gpio_put(PDND_GLITCH, 1);
        for (uint32_t i = 0; i < pulse; ++i) {
          __asm__("NOP");
        }
        gpio_put(PDND_GLITCH_ACTIVE, 0);
        gpio_put(PDND_GLITCH, 0);
      default:
        break;
    }
  }

  return 0;
}