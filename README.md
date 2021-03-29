# Embedded SDK
Provides an example of how to build embedded system simulators with CMake for
hosted operating systems and compilers.

[Changelog](#CHANGES.md)

## Supported Platforms
1. Linux/GCC
2. Linux/Clang
3. Windows/MSVC

## Requirements
**All Platforms**
- Compiler
- CMake
- Git

**Linux**
- `libpcap`
- `pkgconfig`

**Windows**
- `winpcap`

## Selected Libraries
- FreeRTOS
- FreeRTOS+TCP
- FreeRTOS+coreMQTT
- littlefs
- mbedtls

## Default Configuration
By default, Embedded SDK selects some reasonable defaults for desktop simulators.

Additional configuration instructions are under construction.

## Examples
**HelloTask**
2 Tasks periodically printing their names to stdout.

**TcpEchoClientServer**
A client that periodically sends "Hello From Client" to a server.
