# Simplified Secure File Transfer Protocol

Project is WIP, I'll be refining it over time.

This is a barebones version of the Secure File Transfer Protocol (SFTP), which is meant for educational purposes. The OpenSSL library is used for encryption and decryption of files.

Unlike a proper implementation of S-SFTP, this version does not support directories, file attributes, or file permissions. It is also not optimized for performance. The goal of this project is to provide a simple example of how to use the OpenSSL library (or just about anything else) to encrypt and decrypt files. A lot of the code is not exactly what one would call best practice when dealing with files, networking, and security. Emphasis is put on its simplicity.

## Downloading and Compiling

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

For now, a proper build system is not in place. You can compile the code using the following commands in `src/`:

### Server
Compile server with `server.cpp`, `crypto.cpp`, and `logger.cpp`:
```bash
./run.sh server crypto logger
```

### Client
Compile client with `client.cpp`, `crypto.cpp`, and `logger.cpp`:
```bash
./run.sh client crypto logger
```

View [run.sh](src/run.sh) for more details.


# License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.