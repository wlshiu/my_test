#!/usr/bin/env python

import argparse
import wave
import os
import numpy as np


parser = argparse.ArgumentParser(description='Convert a wav file to ASCII header of C language')
parser.add_argument("-i", "--Input", type=str, help="Input wav file")

args = parser.parse_args()

wav_obj = wave.open(args.Input, 'rb')
sample_rate = wav_obj.getframerate()
samples = wav_obj.getnframes()

duration = samples/sample_rate

channels = wav_obj.getnchannels()

print("SampleRate: %d " % sample_rate)
print("samples   : %d " % samples)
print("duration  : %d " % duration)
print("channels  : %d " % channels)

signal_wave = wav_obj.readframes(samples)

pcm_array = np.frombuffer(signal_wave, dtype=np.int16)
# pcm_array = np.frombuffer(signal_wave, dtype=np.uint16)

if channels == 2:
    l_channel = pcm_array[0::2]
    r_channel = pcm_array[1::2]
else:
    l_channel = pcm_array


h_name = "%s.h" % (os.path.basename(args.Input))

with open(h_name,'w') as fout:
    fout.write("#ifdef __cplusplus\nextern \"C\" {\n#endif\n")
    fout.write("#include <stdint.h>\n")
    fout.write("int16_t audio_buffer[] = {\n")

    # np.savetxt(fout, pcm_array, fmt="%d", delimiter=", ", newline=", ")
    np.savetxt(fout, pcm_array, fmt='%d', delimiter=", ", newline=",\n", header=os.path.basename(args.Input), comments= '//')

    fout.write("};\n")
    fout.write("#ifdef __cplusplus\n}\n#endif\n")
