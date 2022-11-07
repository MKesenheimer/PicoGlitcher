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
 * Connections:
 * Glitch (connected via mosfet on pico-pin 19): Connect this to a power line close to the target micro controller.
 * GlitchEn (pico-pin 19; optional): Connect this to external hardware, i.e. a better suitable mosfet.
 * Trig (pico-pin 18; optional): Connect this to the power source of the target board or some other source that can be used to trigger reliably glitching the target board. This is optional. If this pin is not connected, it is pulled up to VCC, and therefore the glitch is started without external trigger.
 * Glitch Monitor (pico-pin 9, pdnd-pin 0; optional): Connect this to an oscilloscope to monitor the glitching process.
 * Power Cycle (pico-pin 10, pdnd-pin 1; optional): use this output to power-cycle the target board. Must be done with additional external hardware.
 * 
 * Details:
 * Glitch: will pull the line to GND after a initial "delay" with a duration of "pulse"
 * GlitchEn: will be enabled if Glitch is pulled to GND.
 * Trig: listens if the device is "ready". The glitch will only be triggered if the device is powered.
 * Glitch Monitor: is enabled during glitching phase (from start of the delay to end of pulse).
 * Power Cycle: can be used to power-cycle the target before every glitch.
 * 
 * Using the glitcher:
 * Option 1: No power-cycling via pdnd board
 * In this case, power-cycling of the target device is not done by the pdnd board, but externally
 * by additional hardware.
 * For example, the pdnd-board is initialized and ready for a glich to start. Then the target
 * board is power cycled by hand or by additional hardware.
 * The glitch starts if the trigger condition is met, i.e. if the target board is powered.
 * Power cycling can be done completely asynchronous to the controller script.
 * 
 * Option 2: Power-cycling is controlled by the pdnd board
 * In this case an additional mosfet or switching unit is controlled by the Power Cycle pin.
 * The target board is switched off for 50ms.
 * The glitch starts if the trigger condition is met, i.e. if the target board is fully powered.
 * Power cycling and glitching is done completely by the pdnd board. This makes the attack faster.
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

// This pin shows, if a glitching attack is currently active.
// Not necessary, but could probably be helpful somehow.
// Note: pdnd-pin 0 corresponds to Raspberry Pi pico pin 9
//#define ENABLE_MONITOR_PIN
#ifdef ENABLE_MONITOR_PIN
const uint PDND_GLITCH_MONITOR = 9;
#endif

// This pin can be used to additionally power-cycle the target.
// Meaning, that before every glitch this pin is pulled to GND for 50ms.
// Note: pdnd-pin 1 corresponds to Raspberry Pi pico pin 10
const uint PDND_POWER_CYCLE = 10;

const uint8_t CMD_DELAY = 0x64;
const uint8_t CMD_PULSE = 0x70;
const uint8_t CMD_GLITCH = 0x67;
const uint8_t CMD_HELLO = 0x68;
const uint8_t CMD_CHECK = 0x63;
const uint8_t CMD_PWR_CYCLING_EN = 0x65;
const uint8_t CMD_PWR_CYCLING_DI = 0x66;

static inline void power_cycle_target() {
  gpio_put(PDND_POWER_CYCLE, 0);
  sleep_ms(50);
  gpio_put(PDND_POWER_CYCLE, 1);
}

void dv(uint32_t delay, uint32_t pulse, bool pwr_cycl_en) {
  cls(false);
  if (pwr_cycl_en)
    pprintf("Glitcher\nD: %lu\nP: %lu\nPwr cycl. enabled.", delay, pulse);
  else
    pprintf("Glitcher\nD: %lu\nP: %lu\nPwr cycl. disabled.", delay, pulse);
}

void glitch(uint32_t delay, uint32_t pulse) {
#ifdef ENABLE_MONITOR_PIN
  gpio_put(PDND_GLITCH_MONITOR, 1);
#endif
  for (uint32_t i = 0; i < delay; ++i) {
    __asm__("NOP");
  }
  gpio_put(PDND_GLITCH, 1);
  for (uint32_t i = 0; i < pulse; ++i) {
    __asm__("NOP");
  }
  gpio_put(PDND_GLITCH, 0);
#ifdef ENABLE_MONITOR_PIN
  gpio_put(PDND_GLITCH_MONITOR, 0);
#endif
}

int main() {
  // init
  stdio_init_all();
  stdio_set_translate_crlf(&stdio_usb, false);
  uint32_t delay = 0, pulse = 0;
  bool pwr_cycl_en = true;

  pdnd_initialize();
  pdnd_initialize_glitcher();
  pdnd_enable_buffers(0);
  pdnd_display_initialize();
  pdnd_enable_buffers(1);
  cls(false);
  pprintf("Glitcher");

  while(1) {
    uint8_t cmd = std::cin.get();
    switch (cmd) {
      case CMD_HELLO:
        cls(false);
        pprintf("Glitcher\nClient connected.");
        break;
      case CMD_DELAY:
        std::cin >> delay;
        dv(delay, pulse, pwr_cycl_en);
        break;
      case CMD_PULSE:
        std::cin >> pulse;
        dv(delay, pulse, pwr_cycl_en);
        break;
      case CMD_PWR_CYCLING_EN:
        pwr_cycl_en = true;
        dv(delay, pulse, pwr_cycl_en);
        break;
      case CMD_PWR_CYCLING_DI:
        pwr_cycl_en = false;
        dv(delay, pulse, pwr_cycl_en);
        break;
      case CMD_GLITCH:
        if (pwr_cycl_en) {
          // power-cycle the target with pdnd-pin 0
          power_cycle_target();
        }
        // wait for external event, monitoring "Trig" pin
        while(!gpio_get(PDND_TRIG));
        glitch(delay, pulse);
        
        // send command back to controller, that glitching is finished.
        // necessary if power-cycling is disabled to synchronize with controller script.
        if (!pwr_cycl_en) {
          std::cout << "x";
        }

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