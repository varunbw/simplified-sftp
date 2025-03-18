"""
    This is a script to generate files of varying sizes, with random data in them
    Meant for performance testing purposes, we do not care about the contents of the files

    This script will generate `num_of_files` files, each of varying sizes
    Currently, it starts at a size of  `1 KB`, and goes upto `num_of_files KB`
    However, you're free to configure it however you want.
"""

import os
import random

def main():
    num_of_files = 15
    for file_size in range(1, num_of_files + 1):
        
        filename = f"perftest_{file_size}KB.txt"
        file_path = os.path.join("./send/", filename)

        with open(file_path, "w") as file:
            data = ''.join(
                random.choices(
                    'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789',
                    k=file_size * 1024
                    )
                )

            file.write(data)

    return


if __name__ == '__main__':
    main()