#!/usr/bin/env python

import argparse
import wave
import matplotlib.pyplot as plt
import numpy as np


parser = argparse.ArgumentParser(description='display wavform of a wav file')
parser.add_argument("-i", "--Input", type=str, help="Input wav file")

args = parser.parse_args()

wav_obj = wave.open(args.Input, 'rb')
sample_rate = wav_obj.getframerate()
samples = wav_obj.getnframes()

duration = samples/sample_rate

channels = wav_obj.getnchannels()

print("freq    : %d " % sample_rate)
print("samples : %d " % samples)
print("duration: %d " % duration)
print("channels: %d " % channels)

signal_wave = wav_obj.readframes(samples)

signal_array = np.frombuffer(signal_wave, dtype=np.int16)

if channels == 2:
    l_channel = signal_array[0::2]
    r_channel = signal_array[1::2]
else:
    l_channel = signal_array

time = np.linspace(0, samples/sample_rate, num=samples)

msg = "sample_rate : %d, samples : %d, duration : %d, channels: %d" % (sample_rate, samples, duration, channels)

plt.figure(figsize=(15, 5))
plt.plot(time, l_channel)

plt.title(msg)
plt.ylabel('Signal Value')
plt.xlabel('Time (s)')
plt.xlim(0, duration)

plt.show()

