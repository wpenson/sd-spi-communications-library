/******************************************************************************/
/**
@file		card_info.ino
@author     Wade Penson
@date		June, 2015
@brief      Outputs the information from the resisters in a card.
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

#include <SPI.h>
#include "sd_spi.h"

#define CHIP_SELECT_PIN 4

void
setup(
)
{
	/*******************************/
	/* Initialization
	/*******************************/

	Serial.begin(115200);

	while (!Serial) {
    	;	/* wait for serial port to connect. Needed for Leonardo only. */
 	}

	int error = 0;

	if (error = sd_spi_init(CHIP_SELECT_PIN)) {
		Serial.print(F("Initialization failed. Error code: "));
		Serial.println(error);
		return;
	}

	Serial.println(F("\n\n\nInitialized successfully!"));

	Serial.print(F("Card type: "));
	switch (sd_spi_card_type()) {
		case 0 : Serial.println(F("Undefined")); break;
		case 1 : Serial.println(F("SD1")); break;
		case 2 : Serial.println(F("SD2")); break;
		case 3 : Serial.println(F("SDHC")); break;
		case 4 : Serial.println(F("MMC")); break;
	}

	/*******************************/
	/* Read registers
	/*******************************/

	sd_spi_cid_t cid;
	if (sd_spi_read_cid_register(&cid)) {
		Serial.print(F("Failed to read cid. Error code: "));
		Serial.println(error);
		return;
	}

	Serial.print(F("\nManufacturer ID: "));
	Serial.print(cid.mid);
	Serial.print(F("\nOEM/Application ID: "));
	Serial.print(cid.oid[0]);
	Serial.print(cid.oid[1]);
	Serial.print(F("\nProduct name: "));
	Serial.print(cid.pnm[0]);
	Serial.print(cid.pnm[1]);
	Serial.print(cid.pnm[2]);
	Serial.print(cid.pnm[3]);
	Serial.print(cid.pnm[4]);
	Serial.print(F("\nProduct revision number: "));
	Serial.print(cid.prv_n);
	Serial.print(F("."));
	Serial.print(cid.prv_m);
	Serial.print(F("\nProduct serial number: "));
	Serial.print((uint32_t)cid.psn_high << 24 | 
			(uint32_t) cid.psn_mid_high << 16 |
			(uint32_t) cid.psn_mid_low << 8 |
			(uint32_t) cid.psn_low);
	Serial.print(F("\nManufacturing Date: Year = 20"));
	Serial.print((cid.mdt_year < 10) ? F("0") : F(""));
	Serial.print(cid.mdt_year);
	Serial.print(F(", Month = "));
	Serial.print(cid.mdt_month);
	Serial.print(F("\n"));

	char *yes = "Yes";
	char *no = "No";

	sd_spi_csd_t csd;
	if (sd_spi_read_csd_register(&csd)) {
		Serial.print(F("Failed to read cid. Error code: "));
		Serial.println(error);
		return;
	}

	Serial.print(F("\nCSD Version: "));
	Serial.print(csd.csd_structure);

	Serial.print(F("\nCard size: "));
	Serial.print((double) sd_spi_card_size() * 512 / 1000000, 0);
	Serial.print(F(" MB"));

	Serial.print(F("\nNumber of blocks: "));
	Serial.print(sd_spi_card_size());

	Serial.print(F("\nRead block partial: "));
	Serial.print((csd.read_bl_partial) ? yes : no);

	Serial.print(F("\nWrite block partial: "));
	Serial.print((csd.write_bl_partial) ? yes : no);

	Serial.print(F("\nRead block misalign: "));
	Serial.print((csd.read_bl_misalign) ? yes : no);

	Serial.print(F("\nWrite block misalign: "));
	Serial.print((csd.write_bl_misalign) ? yes : no);

	Serial.print(F("\nMaximum read block length: "));
	Serial.print(1 << csd.max_read_bl_len);
	Serial.print(F(" bytes"));

	Serial.print(F("\nErase single block enable: "));
	Serial.print((csd.erase_bl_en) ? yes : no);

	Serial.print(F("\nTemporarily write protected: "));
	Serial.print((csd.tmp_write_protect) ? yes : no);

	Serial.print(F("\nPermanently write protected: "));
	Serial.print((csd.perm_write_protect) ? yes : no);

	Serial.print(F("\nCard contents a copy: "));
	Serial.print((csd.copy) ? yes : no);

	Serial.print(F("\nFile format: "));
	if (csd.file_format == 0)
		Serial.print(F("Hard disk-like file system with partition table"));
	else if (csd.file_format == 1)
		Serial.print(F("DOS FAT (floppy-like) with boot sector only (no partition table)"));
	else if (csd.file_format == 2)
		Serial.print(F("Universal File Format"));
	else if (csd.file_format == 3)
		Serial.print(F("Other/Unknown"));
}

void
loop(
)
{

}