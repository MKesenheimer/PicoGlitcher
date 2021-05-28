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

const uint PINGLITCH = 22;
const uint PINGLITCHACTIVE = 26;

const char DELAY = 'd';
const char PULSE = 'p';
const char GLITCH = 'g';

// GPIO Pin to read the Power Voltage from
const int PINVDD = 0;

// TODO
static inline void power_cycle_target() {
  while(!gpio_get(PINVDD));
}

int main() {
  // init
  stdio_init_all();
  gpio_init(PINGLITCH);
  gpio_set_dir(PINGLITCH, GPIO_OUT);
  gpio_init(PINGLITCHACTIVE);
  gpio_set_dir(PINGLITCHACTIVE, GPIO_OUT);
  
  uint32_t delay, pulse;
  while(1) {
    uint8_t cmd = std::cin.get();
    switch (cmd) {
      case DELAY:
        std::cin >> delay;
        //std::cout << "setting delay to " << delay << std::endl;
        break;
      case PULSE:
        std::cin >> pulse;
        //std::cout << "setting pulse to " << pulse << std::endl;
        break;
      case GLITCH:
        std::cout << "delay = " << delay << ", pulse = " << pulse << std::endl;
        //power_cycle_target();
        gpio_put(PINGLITCHACTIVE, 1);
        for (uint32_t i = 0; i < delay; ++i) {
          __asm__("NOP");
        }
        gpio_put(PINGLITCH, 1);
        for (uint32_t i = 0; i < pulse; ++i) {
          __asm__("NOP");
        }
        gpio_put(PINGLITCHACTIVE, 0);
        gpio_put(PINGLITCH, 0);
      default:
        break;
    }
  }

  return 0;
}