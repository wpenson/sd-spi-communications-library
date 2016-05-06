/******************************************************************************/
/**
@file		arduino_platform_dependencies.cpp
@author     Wade H. Penson
@date		June, 2015
@brief      Arduino dependencies needed by the SD SPI library.
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

#include <Arduino.h>
#include <SPI.h>
#include "sd_spi_platform_dependencies.h"

void
sd_spi_pin_mode(
	uint8_t pin,
	uint8_t mode
)
{
	pinMode(pin, mode);
}

void
sd_spi_digital_write(
	uint8_t pin,
	uint8_t state
)
{
	digitalWrite(pin, state);
}

uint32_t
sd_spi_millis(
	void
)
{
	return millis();
}

void
sd_spi_begin(
	void
)
{
	SPI.begin();
}

void
sd_spi_begin_transaction(
	uint32_t transfer_speed_hz
)
{
	SPI.beginTransaction(SPISettings(transfer_speed_hz, MSBFIRST, SPI_MODE0));
}

void
sd_spi_end_transaction(
	void
)
{
	SPI.endTransaction();
}

void
sd_spi_send_byte(
	uint8_t b
)
{
	SPI.transfer(b);
}

uint8_t
sd_spi_receive_byte(
	void
)
{
	return SPI.transfer(0xFF);
}