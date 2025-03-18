cmake_minimum_required(VERSION 3.10)

# Place executables in the project root
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}")

# Set the project name
project(SimplifiedSFTP)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(include)

# Add debug info
add_compile_options(-g)

# Add the OpenSSL library
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

# Add the executable for the receiver
add_executable(receiver.out
    src/receiver.cpp
    src/crypto.cpp
    src/logger.cpp
)

# Link the OpenSSL library to the receiver executable
target_link_libraries(receiver.out ${OPENSSL_LIBRARIES})

# Add the executable for the sender
add_executable(sender.out
    src/sender.cpp
    src/crypto.cpp
    src/logger.cpp
)

# Link the OpenSSL library to the sender executable
target_link_libraries(sender.out ${OPENSSL_LIBRARIES})