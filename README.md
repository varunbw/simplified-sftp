# Simplified Secure File Transfer Protocol

Project is WIP, I'll be refining it over time.

This is a barebones version of the Secure File Transfer Protocol (SFTP), which is meant for educational purposes. The OpenSSL library is used for encryption and decryption of files.

Unlike a proper implementation of S-SFTP, this version does not support directories, file attributes, or file permissions. It is also not optimized for performance. The goal of this project is to provide a simple example of how to use the OpenSSL library (or just about anything else) to encrypt and decrypt files. A lot of the code is not exactly what one would call best practice when dealing with files, networking, and security. Emphasis is put on its simplicity.

Even for a "simplfied" version, the source code might look complex to some (aah C++). However, most of it is just error handling, and explanatory comments. If you remove those, its actually quite simple.

## Prerequisites
- CMake
- OpenSSL library

## Downloading and Compiling

CMake is used to compile the code. The following instructions are for Ubuntu 22.04, but should work on any Linux distribution with minor modifications. If you don't have CMake installed, you can install it with:
```bash
sudo apt-get install cmake
```

### Steps

1. Clone the repository:
```bash
git clone https://github.com/varunbw/simplified-sftp.git
cd simplified-sftp
```

2. Install OpenSSL
```bash
sudo apt-get install libssl-dev
```

3. Compile and run the code

```bash
mkdir build && cd build
cmake ..
make
```

## Running the program

1. Run the server:
```bash
./receiver.out <file_name_to_save_as>
```

2. Run the client:
```bash
./sender.out <file_name_to_send>
```

## Configuration

You can configure the server and client by modifying the source code to change the port number and IP address. The default port is 8080 and the default IP address is localhost.

The program uses a hardcoded key and IV for encryption and decryption. You can change these values in the [crypto.hpp](include/crypto.hpp) file.

# License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.