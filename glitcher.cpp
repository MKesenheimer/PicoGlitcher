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
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/irq.h"

const uint PINGLITCH = 22;
const uint PINGLITCHACTIVE = 26;
// GPIO Pin to read the Power Voltage from
const int PINVDD = 0;

const char DELAY = 'd';
const char PULSE = 'p';
const char GLITCH = 'g';
const char CHECK = 'c';

static uint32_t delay, pulse;
static bool active;

// TODO
static inline void power_cycle_target() {
  while(!gpio_get(PINVDD));
}

void glitch(uint32_t delay, uint32_t pulse) {
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
}

// Core 1 Main Code
void core1_entry() {
  save_and_disable_interrupts();
  while (true) {
    if (active) {
      glitch(delay, pulse);
      // TODO: check if glitch was successful
    }
  }
}


int main() {
  // init
  stdio_init_all();
  gpio_init(PINGLITCH);
  gpio_set_dir(PINGLITCH, GPIO_OUT);
  gpio_init(PINGLITCHACTIVE);
  gpio_set_dir(PINGLITCHACTIVE, GPIO_OUT);

  // Start glitching on core 1
  //multicore_launch_core1(core1_entry);
  

  active = false;
  while(1) {
    uint8_t cmd = std::cin.get();
    switch (cmd) {
      case DELAY:
        gpio_put(PINGLITCHACTIVE, 0);
        std::cin >> delay;
        break;
      case PULSE:
        std::cin >> pulse;
        break;
      case GLITCH:
        glitch(delay, pulse);
        //active = true;
        break;
      case CHECK:
        std::cout << "delay = " << delay << ", pulse = " << pulse << std::endl;
        break;
      default:
        break;
    }
  }

  return 0;
}