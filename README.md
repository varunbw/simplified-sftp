# Simplified Secure File Transfer Protocol

This is a barebones version of the Secure File Transfer Protocol (SFTP), which is meant for educational purposes. The OpenSSL library is used for encryption and decryption of files. The project is more about the source code, than the final product. I have aimed to demonstrate how to use the OpenSSL library to encrypt and decrypt files, and how to transfer files over a network using sockets. The code is not production-ready and should not be used in a real-world application without significant modifications.

Unlike a proper implementation of S-SFTP, this version does not support directories, file attributes, or file permissions. It is also not optimized for performance. The goal of this project is to provide a simple example of how to use the OpenSSL library (or just about anything else) to encrypt and decrypt files. A lot of the code is not exactly what one would call best practice when dealing with files, networking, and security. Emphasis is put on its simplicity.

## Prerequisites
- CMake
- OpenSSL library

## Downloading and Compiling

CMake is used to compile the code. The following instructions are for Ubuntu 22.04, but should work on any Linux distribution with minor modifications. If you don't have CMake installed, you can install it with:
```bash
sudo apt-get install cmake
```

### Steps

TLDR: There is a Quick Setup block at the end of this README file with all the setup commands required. You can copy paste that directly if you wish. It will install CMake and OpenSSL, clone the repository, and compile the code.

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

The executables are placed in the root directory of the project. You can run the server and client from there.

Note: Ensure that the OpenSSL library is installed and accessible in your system's library path.

## Running the program

1. Run the server:
```bash
./receiver.out [-f <file_name_to_receive> | -n <number_of_files_to_receive>]
```

- `-f` Specify name of file to save as
- `-n` Specify number of files to receive, this is batch processing, see [receiver.cpp](src/receiver.cpp) for more details.

2. Run the client:
```bash
./sender.out [-f <file_name_to_send> | -n <number_of_files_to_send>]
```
- `-f` Specify name of file to send
- `-n` Specify number of files to send, this is batch processing, see [sender.cpp](src/sender.cpp) for more details.

## Configuration

You can configure the server and client by modifying the source code to change the port number and IP address. The default port is 8080 and the default IP address is localhost.

The program uses a hardcoded key and IV for encryption and decryption. You can change these values in the [crypto.hpp](include/crypto.hpp) file.

# License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

# Quick Setup

You can copy paste this block of commands to setup the project quickly. It will install CMake and OpenSSL, clone the repository, and compile the code.

```bash
sudo apt-get install cmake libssl-dev -y
git clone https://github.com/varunbw/simplified-sftp.git
cd simplified-sftp
mkdir build && cd build
cmake ..
make
```