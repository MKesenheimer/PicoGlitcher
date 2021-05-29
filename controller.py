#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Controller for the Raspberry Pi voltage glitcher.
Author: Matthias Kesenheimer (m.kesenheimer@gmx.net)
Copyright: Copyright 2021, Matthias Kesenheimer
License: GPL
Version: 0.7
"""

import serial
import threading
import time
import argparse

class listening(threading.Thread):
  """listening class capable of threading."""

  def __init__(self, ser):
    """class initializer"""
    self.__stopEvent = threading.Event()
    self.__ser = ser
    super().__init__()

  def read(self):
    """function to read data from the serial port"""
    data = str("")
    while self.__ser.in_waiting > 0:
      payload = self.__ser.read(self.__ser.in_waiting)
      data = payload.decode("ascii")
    return data

  def run(self):
    """main thread"""
    while not self.__stopEvent.isSet():
      data = self.read()
      print(data, end='')
      time.sleep(0.1)

  def join(self, timeout=None):
    """set stop event and join within a given time period"""
    self.__stopEvent.set()
    print("stopping listening thread.")
    super().join(timeout)

if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="Control the Glitcher over serial")
  parser.add_argument('port', metavar='SERIALPORT', type=str, nargs=1,
                      help='serial port to connect to')
  parser.add_argument('--baudrate', dest='baudrate', type=int, nargs=1, default=[115200],
                      help='sum the integers (default: find the max)')
  parser.add_argument('--timeout', dest='timeout', type=float, nargs=1, default=[0.1],
                      help='Timeout of the serial connection')
  parser.add_argument('-d', dest='delay', type=int, nargs=3, default=[1,100,1],
                      help='delay of the pulse')
  parser.add_argument('-p', dest='pulse', type=int, nargs=3, default=[1,100,1],
                      help='pulse width')
  args = parser.parse_args()

  ser = serial.Serial(port=args.port[0], baudrate=args.baudrate[0], timeout=args.timeout[0])

  x = listening(ser)
  x.start()
  time.sleep(args.timeout[0])

  try:
    while True:
      for d in range(args.delay[0], args.delay[1] + 1, args.delay[2]):
        for p in range(args.pulse[0], args.pulse[1] + 1, args.pulse[2]):
          time.sleep(args.timeout[0])
          bts = bytes("d {}\n".format(d), "ascii")
          ser.write(bts)
          bts = bytes("p {}\n".format(p), "ascii")
          ser.write(bts)
          ser.write(b"g\n")
          #ser.write(b"c\n")
          # TODO
          #if testDevice(): # JTAG etc
          #  print("SUCCESS")
          #  exit(0)

  except (KeyboardInterrupt, SystemExit):
    x.join(args.timeout[0])
    print("\nGood bye.")
    exit(0)