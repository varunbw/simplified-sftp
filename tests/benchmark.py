import matplotlib.pyplot as plt
import pandas as pd
import re

pattern_map = {
    'file_size':          re.compile(r'^SendFile\(\): Size of file: (\d+) bytes'),
    'loading_file_time':  re.compile(r'^SendFile\(\):\s+Loading file - ([\d.]+) ms'),
    'encryption_time':    re.compile(r'^EncryptAndSend\(\):\s+Encryption - ([\d.]+) ms'),
    'enc_file_size':      re.compile(r'^EncryptAndSend\(\): Size of encrypted data: (\d+) bytes'),
    'sending_file_time':  re.compile(r'^EncryptAndSend\(\):\s+Sending file - ([\d.]+) ms'),
    'hash_calc_time':     re.compile(r'^CalculateHashAndSend\(\): Hash calculation - ([\d.]+) ms'),
    'hash_send_time':     re.compile(r'^CalculateHashAndSend\(\):\s+Sending hash - ([\d.]+) ms'),
    'total_time':         re.compile(r'^SendFile\(\): Complete sending operation\s+- ([\d.]+) ms')
}

entries = []
current = {}

with open("logs/send.txt", "r", encoding="utf-8") as f:
    for line in f:
        for key, pattern in pattern_map.items():
            match = pattern.search(line.strip())
            if match:
                if 'size' not in key:
                    current[key] = float(match.group(1))
                else:
                    current[key] = int(match.group(1))

        # If total_time found, we consider this block complete
        if 'total_time' in current:
            entries.append(current)
            current = {}


df = pd.DataFrame(entries)
print(df)

times = [
    "loading_file_time",
    "encryption_time",
    "sending_file_time",
    "hash_calc_time",
    "hash_send_time",
    "total_time"
]

for t in times:
    plt.plot(df["file_size"], df[t], label=t)

plt.xlabel("File Size (bytes)")
plt.ylabel("Time (ms)")
plt.legend()
plt.show()