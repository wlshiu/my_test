#!/usr/bin/env python

import sys
import argparse
import numpy as np
import matplotlib.pyplot as plt

'''
$ ./z_pwm_render.py -i ./pwm.csv -ch 6
'''

parser = argparse.ArgumentParser(description='Render pwm log to waveform')
parser.add_argument("-o", "--Output", type=str, help="output waveform")
parser.add_argument("-i", "--Input", type=str, help="input pwm log file")
parser.add_argument("-s", "--Samples", type=str, help="Samples of 1-period")
parser.add_argument("-ch", "--Channels", type=str, help="input channel count (in pwm log)")

args = parser.parse_args()

'''
pwm log
ch1, ch2, ch3, ... chX
---- pwm.log values format -----
  1,   0,   0,
  1,   0,   0,
  1,   0,   0,
  1,   0,   0,
  0,   0,   0,
  0,   0,   0,
  ...
'''

if not args.Input:
    print('No input parameter ...')
    sys.exit(1)

if not args.Samples:
    samples_1_period = 100
else:
    samples_1_period = int(args.Samples)

if not args.Channels:
    pwm_channle_max = 3
else:
    pwm_channle_max = int(args.Channels)


cycles = []

pwm_channels = [[] for i in range(pwm_channle_max)]

T = 0

with open(args.Input, 'r') as in_file:

    while True:
        T = T + 1
        line = in_file.readline()
        if not line:
            break;

        items = line.split(', ')

        cycles.append(T)

        for i in range(pwm_channle_max):
            pwm_channels[i].append(int(items[i]) - i * 2)


fig, ax = plt.subplots()
for i in range(pwm_channle_max):
    plt.plot(cycles, pwm_channels[i], label='PWM-' + str(i))

period_cnt = [x for x in range(0, T, samples_1_period)]
ax.set_xticks(period_cnt)
plt.grid(axis='x', color = 'b')

plt.legend(bbox_to_anchor=(1.05, 1.0), loc='upper left')
plt.tight_layout()

plt.savefig('PWM_Waveform.jpg')
plt.show()
