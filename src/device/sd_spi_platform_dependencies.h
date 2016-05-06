/******************************************************************************/
/**
@file		sd_spi_platform_dependencies.cpp
@author     Wade Penson
@date		June, 2015
@brief      Interface for the dependencies needed by the SD SPI library.
@copyright  Copyright 2015 Wade Penson

@license    Licensed under the Apache License, Version 2.0 (the "License");
            you may not use this file except in compliance with the License.
            You may obtain a copy of the License at

              http://www.apache.org/licenses/LICENSE-2.0

            Unless required by applicable law or agreed to in writing, software
            distributed under the License is distributed on an "AS IS" BASIS,
            WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or 
            implied. See the License for the specific language governing
            permissions and limitations under the License.
*/
/******************************************************************************/

#if !defined(SD_SPI_PLATFORM_DEPENDENCIES_H_)
#define SD_SPI_PLATFORM_DEPENDENCIES_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>

#if !defined(INPUT)
#define INPUT	0
#endif

#if !defined(OUTPUT)	
#define OUTPUT 	1
#endif

#if !defined(LOW)
#define LOW 	0
#endif

#if !defined(HIGH)
#define HIGH 	1
#endif

void
sd_spi_pin_mode(
	uint8_t pin,
	uint8_t mode
);

void
sd_spi_digital_write(
	uint8_t pin,
	uint8_t state
);

uint32_t
sd_spi_millis(
	void
);

void
sd_spi_begin(
	void
);

void
sd_spi_begin_transaction(
	uint32_t transfer_speed_hz
);

void
sd_spi_end_transaction(
	void
);

void
sd_spi_send_byte(
	uint8_t b
);

uint8_t
sd_spi_receive_byte(
	void
);

#if defined(__cplusplus)
}
#endif

#endif /* SD_SPI_PLATFORM_DEPENDENCIES_H_ */