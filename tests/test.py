import os
import filecmp

send_dir = "send"
recv_dir = "recv"

send_files = [f
    for f in os.listdir(send_dir)
        if f.startswith("perftest_") and f.endswith(".txt")
]

for filename in send_files:
    send_path = os.path.join(send_dir, filename)
    recv_path = os.path.join(recv_dir, filename)
    
    if os.path.exists(recv_path) == False:
        print(f"Missing file in recv/: {filename}")

    else:
        if filecmp.cmp(send_path, recv_path, shallow=False):
            print(f"\033[92m{filename} matches\033[0m")
        else:
            print(f"\033[91m{filename} does not match\033[0m")