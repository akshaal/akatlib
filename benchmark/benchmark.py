#!/usr/bin/env python

from simavr import *
import unittest
import sys

name = sys.argv [3]
mode = sys.argv [2]
mcu = sys.argv [1]
freq = 8000000

f = open('result-' + mcu, 'aw')

# Run
avr = AVR (filename = mode + "-" + name + ".avr", mcu = mcu, freq = freq, quiet = True)
stop = False
timings = list ()
begin = None

def on_signal (value, arg):
    if value == 0:
        global begin
        begin = avr.cycle
    elif begin != None:
        timings.append (avr.cycle - begin)

def on_exit (value, arg):
    global stop
    if value != 0:
        stop = True

avr.get_ioport_irq ('B', 0).register_notify (on_signal)
avr.get_ioport_irq ('B', 1).register_notify (on_exit)

while not stop:
    avr.run_cycles ()

print >> f, mode + "   " + name + ": size = " + str(avr._firmware.flashsize) + ", timings = " + str(timings).replace("L", "")
