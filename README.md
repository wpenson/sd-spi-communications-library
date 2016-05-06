# SD SPI Communication Library
SD cards have support to communicate with devices that have a Serial Peripheral Interface (SPI) bus. This allows microcontrollers to read from and write to SD cards. The cards use specific commands to communicate with the host. The purpose of this library is to abstract this communication layer from the user.

This library supports reading and writing blocks to and from the card, erasing blocks, and getting information about the card (the information in the registers). It also provides methods for sequentially reading from and writing to the card; the card does some type of magic to speed up these sequential operations when explicitly called.

Optionally, there is a single 512 byte buffer in the library that can be used. Since blocks of size 512 bytes are the smallest accessible unit for reading and writing on the cards, a buffer is needed if you want to write out less than 512 bytes to a block or read from the same block multiple times without reading it more than once.

## Usage
If you are using the Arduino IDE, you will need to put all of the source files into a single folder with your `.ino' file. You also need to include the Arduino SPI library in your `.ino` file (#include <SPI.h>).

If you are not using Arduino, you must add the SDI support code for your platform in the `spi_communication.c` file. You can then compile the library using CMake.

PlanckUnit is a submodule so...

## Features
- Supports MMC, SD1, SD2, and SDHC/SDXC cards
- Out of the box support for Arduino using the Arduino SPI library
- Easy to extend it to other platforms
- Read from and write to blocks
- There is an interface for the faster sequential reading and writing provided by the SD communication layer.
- An optional block buffer that makes reading and writing simple
- Read information from the CSD and CID registers
- A `.ino` is provided which outputs all the information from the registers on a card such as the size of a card and the manufacturer.
- An emulator for the library which uses a file instead of an SD card is included so that code can be debugged off-device.

## TODOs
- Communicate with multiple SD cards on the same SPI bus
- Allow for block sizes larger than 512 bytes
- Support for more than one block buffer
- Improve the unit tests
- Add the Doxygen generator configuration

## Support
If there are any bugs, create a new issue with a detailed description of your bug. If you have any questions or suggestions, please send me an email at wadepenson@gmail.com.

## Author
Wade Penson

wadepenson@gmail.com