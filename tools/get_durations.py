import os
import struct

def get_duration(filepath):
    with open(filepath, 'rb') as f:
        data = f.read(200)
    fmt_pos = data.find(b'fmt ')
    data_pos = data.find(b'data')
    if fmt_pos < 0 or data_pos < 0:
        return -1
    channels = struct.unpack('<H', data[fmt_pos+10:fmt_pos+12])[0]
    sample_rate = struct.unpack('<I', data[fmt_pos+12:fmt_pos+16])[0]
    bits = struct.unpack('<H', data[fmt_pos+22:fmt_pos+24])[0]
    data_size = struct.unpack('<I', data[data_pos+4:data_pos+8])[0]
    samples = data_size / (channels * (bits / 8))
    return samples / sample_rate

path = r'c:\Windows.old\Users\chris\AndroidStudioProjects\Pinball-on-Android\app\src\main\assets'
files = [f for f in os.listdir(path) if f.upper().startswith('SOUND') and f.upper().endswith('.WAV')]
files.sort(key=lambda x: int(''.join(filter(str.isdigit, x)) or 0))
for f in files:
    dur = get_duration(os.path.join(path, f))
    print(f'{f}: {dur:.6f}')
