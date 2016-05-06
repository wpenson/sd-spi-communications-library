# SD SPI Communication Library
SD cards have support to communicate with devices that have a Serial Peripheral Interface (SPI) bus. This allows microcontrollers to read from and write to SD cards. The cards use specific commands to communicate with the host. The purpose of this library is to abstract this communication layer from the user.

This library supports reading and writing blocks to and from the card, erasing blocks, and getting information about the card (from the information in the registers). It also provides methods for sequentially reading from and writing to the card; the card has an explicit command that uses some type of magic to speed up these sequential operations.

Optionally, there is a single 512 byte buffer in the library that can be used. Since blocks of size 512 bytes are the smallest accessible unit for reading and writing on the cards, a buffer is needed if you want to write out less than 512 bytes to a block or read from the same block multiple times without reading it more than once.

## Features
- Supports MMC, SD1, SD2, and SDHC/SDXC cards
- Out of the box support for Arduino using the Arduino SPI library
- Easy to extend it to other platforms
- Read from and write to blocks
- Functions for the faster sequential reading and writing provided by the SD communication layer
- Optional block buffer that makes reading and writing simple
- Read information from the CSD and CID registers
- A `.ino` is provided which outputs all the information from the registers on a card such as the size of a card and the manufacturer
- Emulator for the library which uses a file instead of an SD card is included so that code can be debugged off-device

## Usage
If you are using the Arduino IDE, you will need to put all of the source files into a single folder with your `.ino` file. There is a python script provided called `arduino_flattener.py` that will put the files in a folder for you. You also need to include the Arduino SPI library in your `.ino` file (put `#include <SPI.h>` at the top).

If you are not using Arduino, you must add the support code for your platform in the `sd_spi_platform_dependencies.c` file. You need to add code for SPI, timing, and toggling the chip select pin. You can then compile the library by modify the CMake provided or using your own build tool.

If you wish to run the unit tests, you must clone the project with `git clone --recursive` or run `git submodule init` then `git submodule update` after cloning to retrieve the code for the PlanckUnit submodule. The unit test can easily be ran without Arduino. If you have `printf()` support, you just need to call `runalltests_sd_spi()`.

## Examples
#### Simple Read/Write
```C
#include <SPI.h>
#include "sd_spi.h"

#define CHIP_SELECT_PIN 4

void setup() {
    Serial.begin(115200);

	while (!Serial) {;}

    if (sd_spi_init(CHIP_SELECT_PIN) == SD_ERR_OK) {
        Serial.println("Successfully initialized the card!")
    }
    
    // Write 2 bytes to block 100 at byte 20 within that block. Read works in the same fashion.
    int num = 31416;
    if (sd_spi_write(100, &num, 2, 20) != SD_ERR_OK) {
        Serial.println("Write failed.")
    }
    
    // Data is not written out to the device until a different block is read or until we explicitly call flush.
    if (sd_spi_flush() != SD_ERR_OK) {
        Serial.println("Write failed.")
    }
}

void loop() {}
```

#### Sequential Read/Write
```C
#include <SPI.h>
#include "sd_spi.h"

#define CHIP_SELECT_PIN 4

void setup() {
    // Error checking is left out for brevity.
    sd_spi_init(CHIP_SELECT_PIN);
    
    int start_block_address = 0;
    int num_blocks_pre_erase = 2; // If you know how many blocks you need, you can specify how many you would like pre-erased to speed up writing.
    
    // Initialize the process.
    sd_spi_write_continuous_start(start_block_address, num_blocks_pre_erase);
    
    int num = 31416;
    sd_spi_write_continuous(&num, 2, 0); // Write out 2 bytes of data to the first block in the sequence at the first byte in the block.
    sd_spi_write_continuous_next(); // Advance to the next block in the sequence.
    sd_spi_write_continuous(&num, 2, 0) // Write out 2 bytes of data to the second block in the sequence at the first byte in the block.
    sd_spi_write_continuous_stop(); // Stop the writing process. This implicitly flushes the buffer to the card.
}

void loop() {}
```

## TODOs
- Communicate with multiple SD cards on the same SPI bus
- Allow for block sizes larger than 512 bytes
- Support for more than one block buffer
- Improve the unit tests
- Add the Doxygen generator configuration

## Support
If there are any bugs, create a new issue with a detailed description of your bug. If you have any questions or suggestions, please send me an email. Feel free to contribute to this repository. Any improvements will be greatly appreciated.

## Author
Wade Penson

wadepenson@gmail.com
