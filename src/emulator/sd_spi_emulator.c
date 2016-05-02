/******************************************************************************/
/**
@file		sd_spi_emulator.h
@author     Wade H. Penson
@date		June, 2015
@brief      SD SPI emulated library implementation.
@copyright  Copyright 2015 Lawrence, Fazackerley, Douglas, Huang, Penson.

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

#include "sd_spi_emulator.h"
#include "./../sd_spi_commands.h"

uint8_t 	sd_spi_dirty_write 	= 0;
uint32_t	num_reads 			= 0;
uint32_t	num_writes 			= 0;

/* An sd_spi_card_t structure for internal state. */
static sd_spi_card_t card;

/**
@brief		Clears the buffer and sets the values to 0.

@param[in]	card	An sd_spi_card_t which has been initialized.

@return		An error code as defined by one of the SD_ERR_* definitions.
*/
static void
sd_spi_clear_buffer(
	void
);

/**
@brief		Performs the direct write to the card.

@param		block_address		The address of the block on the card.
@param[in]	data				An array of data / an address to the data in
								memory.
@param		number_of_bytes		The size of the data in bytes.
@param		byte_offset			The byte offset of where to start writing in the
								block.

@return		An error code as defined by one of the SD_ERR_* definitions.	
*/
static int8_t
sd_spi_write_out_data(
	uint32_t	block_address,
	void	 	*data,
	uint16_t 	number_of_bytes,
	uint16_t 	byte_offset
);

/**
@brief		Performs the direct read from the card.

@param		block_address		The address of the block on the card.
@param[out]	data_buffer			A location in memory to write the data to.
@param		number_of_bytes		The number of bytes to read.
@param		byte_offset			The byte offset of where to start reading in the
								block.

@return		An error code as defined by one of the SD_ERR_* definitions.
*/
static int8_t
sd_spi_read_in_data(
	uint32_t 	block_address,
	void	 	*data_buffer,
	uint16_t 	number_of_bytes,
	uint16_t 	byte_offset
);

/**
@brief	Asserts the chip select pin for the card.
*/
static void
sd_spi_select_card(
	void
);

/**
@brief	De-asserts the chip select pin for the card.
*/
static void
sd_spi_unselect_card(
	void
);

int8_t
sd_spi_init(
	uint8_t chip_select_pin
)
{
	sd_spi_select_card();

	card.spi_speed = 1;
	card.card_type = 3;
	card.chip_select_pin = chip_select_pin;
	card.is_chip_select_high = 1;
	card.is_read_write_continuous = 0;
	card.continuous_block_address = 0;

#if defined(BUFFER)
	card.buffered_block_address = 0;
	card.is_buffer_current = 0;
	card.is_buffer_written = 1;
#endif

	if ((card.fp = fopen("data.raw", "rb+")) == NULL)
	{
		if ((card.fp = fopen("data.raw", "wb+")) == NULL)
		{
			return SD_ERR_INIT_TIMEOUT;
		}
	}

	if (fseek(card.fp, 0, SEEK_END) != 0)
	{
		return SD_ERR_INIT_TIMEOUT;
	}

	int32_t size;

	if ((size = ftell(card.fp)) == -1)
	{
		return SD_ERR_INIT_TIMEOUT;
	}

	if (size < (sd_spi_card_size() << 9))
	{
		rewind(card.fp);

		uint64_t current_byte;
		char val = 0;
		
		for (current_byte = 0;
			 current_byte < (sd_spi_card_size() << 9);
			 current_byte++)
		{
			fwrite(&val, 1, 1, card.fp);
		}

		if (fflush(card.fp) != 0)
		{
			return SD_ERR_INIT_TIMEOUT;
		}
	}

	if (fclose(card.fp) != 0)
	{
		return SD_ERR_INIT_TIMEOUT;
	}

	uint16_t i;
	for (i = 0; i < 512; i++)
    {
    	card.sd_spi_buffer[i] = 0;
    }

	sd_spi_unselect_card();
	return SD_ERR_OK;
}

int8_t
sd_spi_write(
	uint32_t 	block_address,
	void	 	*data,
	uint16_t 	number_of_bytes,
	uint16_t 	byte_offset
)
{
	/* Check to make sure that data is within page bounds. */
	if (number_of_bytes + byte_offset > 512)
	{
		return SD_ERR_WRITE_OUTSIDE_OF_BLOCK;
	}

#if defined(SD_SPI_BUFFER)
	/* Write a whole block out if it is 512 bytes, otherwise read block into
	   buffer for partial writing. */
	if (number_of_bytes == 512 && !card.is_read_write_continuous)
	{
		int8_t response = sd_spi_write_block(block_address, data);
		sd_spi_unselect_card();

		return response;
	}
	else if (!card.is_read_write_continuous && (card.buffered_block_address !=
			 block_address || !card.is_buffer_current))
	{
		int8_t response;

		if ((response = sd_spi_flush()))
		{
			return response;
		}

		if (sd_spi_dirty_write)
		{
			card.buffered_block_address = block_address;
		}
		else
		{
			if ((response = sd_spi_read_in_data(block_address, card.sd_spi_buffer,
												512, 0)))
			{
				return response;
			}
		}
	}

	memcpy(card.sd_spi_buffer + byte_offset, data, number_of_bytes);
	card.is_buffer_written = 0;

	return SD_ERR_OK;
#else
	int8_t response = sd_spi_write_out_data(block_address, data,
											number_of_bytes, byte_offset);

	sd_spi_unselect_card();
	return response;
#endif
}

int8_t
sd_spi_write_block(
	uint32_t 	block_address,
	void	 	*data
)
{
	int8_t response;

#if defined(SD_SPI_BUFFER)
	if ((response = sd_spi_flush()))
	{
		return response;
	}
#endif

	response = sd_spi_write_out_data(block_address, data, 512, 0);

#if defined(SD_SPI_BUFFER)
	if (!card.is_read_write_continuous || block_address == card.continuous_block_address) // TODO: Does this logic make sense?
	{
		memcpy(card.sd_spi_buffer, data, 512);
	}

	card.is_buffer_written = 1;
	card.buffered_block_address = block_address;
#endif

	sd_spi_unselect_card();
	return response;
}

int8_t
sd_spi_flush(
	void
)
{
#if defined(SD_SPI_BUFFER)
	if (card.is_buffer_written)
	{
		return SD_ERR_OK;
	}

	int8_t response;
	if ((response = sd_spi_write_out_data(card.buffered_block_address,
										  card.sd_spi_buffer, 512, 0)))
	{
		return response;
	}

	if (!card.is_read_write_continuous)
	{
		card.is_buffer_current = 1;
	}
	
	card.is_buffer_written = 1;
	sd_spi_unselect_card();
#endif

	return SD_ERR_OK;
}

int8_t
sd_spi_write_continuous_start(
	uint32_t start_block_address,
	uint32_t num_blocks_pre_erase
)
{
#if defined(SD_SPI_BUFFER)
	int8_t response;
	if ((response = sd_spi_flush()))
	{
		return response;
	}
#endif

	/* Keep track of block address for error checking and buffering. */
	card.continuous_block_address = start_block_address;
	card.is_read_write_continuous = 1;

#if defined(SD_SPI_BUFFER)
	sd_spi_clear_buffer();
	card.buffered_block_address = card.continuous_block_address;
#endif

	sd_spi_unselect_card();
	return SD_ERR_OK;
}

int8_t
sd_spi_write_continuous(
	void		*data,
	uint16_t 	number_of_bytes,
	uint16_t 	byte_offset
)
{
	return sd_spi_write(card.continuous_block_address, data,
						number_of_bytes, byte_offset);
}

int8_t
sd_spi_write_continuous_next(
	void
)
{
#if defined(SD_SPI_BUFFER)
	int8_t response = sd_spi_flush();
	sd_spi_clear_buffer();

	return response;
#else
	return SD_ERR_OK;
#endif
}

int8_t
sd_spi_write_continuous_stop(
	void
)
{
#if defined(SD_SPI_BUFFER)
	/* Flush buffer if it hasn't been written. */
	int8_t response;
	if ((response = sd_spi_flush()))
	{
		return response;
	}
#endif

	sd_spi_select_card();
	card.is_read_write_continuous = 0;

	return sd_spi_card_status();
}

int8_t
sd_spi_read(
	uint32_t 	block_address,
	void	 	*data_buffer,
	uint16_t 	number_of_bytes,
	uint16_t 	byte_offset
)
{
#if defined(SD_SPI_BUFFER)
	int8_t response;

	if ((response = sd_spi_flush()))
	{
		return response;
	}

	/* Check to make sure that data is within page bounds. */
	if (number_of_bytes + byte_offset > 512)
	{
    	return SD_ERR_READ_OUTSIDE_OF_BLOCK;
  	}

	if (card.buffered_block_address != block_address ||
		!card.is_buffer_current)
	{
		/* Read block into buffer. */
		if ((response = sd_spi_read_in_data(block_address, card.sd_spi_buffer,
											512, 0)))
		{
			return response;
		}
	}

	memcpy(data_buffer, card.sd_spi_buffer + byte_offset, number_of_bytes);

	return SD_ERR_OK;
#else
	return sd_spi_read_in_data(block_address, data_buffer, number_of_bytes,
						   	   byte_offset);
#endif
}

int8_t
sd_spi_read_continuous_start(
	uint32_t 	start_block_address
)
{
#if defined(SD_SPI_BUFFER)
	int8_t response;
	if ((response = sd_spi_flush()))
	{
		return response;
	}
#endif

	card.continuous_block_address = start_block_address;
	card.is_read_write_continuous = 1;

#if defined(SD_SPI_BUFFER)
	if ((response = sd_spi_read_in_data(card.continuous_block_address,
										card.sd_spi_buffer, 512, 0)))
	{
		sd_spi_unselect_card();
		return response;
	}
#endif

	sd_spi_unselect_card();
	return SD_ERR_OK;
}

int8_t
sd_spi_read_continuous(
	void	 	*data_buffer,
	uint16_t 	number_of_bytes,
	uint16_t 	byte_offset
)
{
#if defined(SD_SPI_BUFFER)
	return sd_spi_read(card.buffered_block_address, data_buffer,
					   number_of_bytes, byte_offset);
#else
	return sd_spi_read_in_data(card.continuous_block_address, data_buffer,
							   number_of_bytes, byte_offset);
#endif
}

int8_t
sd_spi_read_continuous_next(
	void
)
{
#if defined(SD_SPI_BUFFER)
	return sd_spi_read_in_data(card.continuous_block_address,
							   card.sd_spi_buffer, 512, 0);
#else
	return SD_ERR_OK;
#endif
}

int8_t
sd_spi_read_continuous_stop(
	void
)
{
	sd_spi_select_card();
	card.is_read_write_continuous = 0;
	sd_spi_unselect_card();

	return SD_ERR_OK;
}

int8_t
sd_spi_erase_all(
	void
)
{
	return sd_spi_erase_blocks(0, sd_spi_card_size());
}

int8_t
sd_spi_erase_blocks(
	uint32_t start_block_address,
	uint32_t end_block_address
)
{
#if defined(SD_SPI_BUFFER)
	int8_t response;
	if ((response = sd_spi_flush()))
	{
		return response;
	}

	if (card.buffered_block_address > start_block_address &&
		card.buffered_block_address < end_block_address)
	{
		sd_spi_clear_buffer();
		card.is_buffer_written = 1;
		card.is_buffer_current = 1;
	}
#endif

	if ((card.fp = fopen("data.raw", "rb+")) == NULL)
	{
		return SD_ERR_ERASE_FAILURE;
	}

	if (fseek(card.fp, start_block_address << 9, SEEK_SET) != 0)
	{
		return SD_ERR_ERASE_FAILURE;
	}

	char val = 0;
	uint64_t current_byte;
	for (current_byte = (start_block_address << 9);
		 current_byte < end_block_address << 9;
		 current_byte++)
	{
		fwrite(&val, 1, 1, card.fp);
	}

	if (fflush(card.fp) != 0)
	{
		return SD_ERR_ERASE_FAILURE;
	}

	if (fclose(card.fp) != 0)
	{
		return SD_ERR_ERASE_FAILURE;
	}

	sd_spi_unselect_card();
	return SD_ERR_OK;
}

uint32_t
sd_spi_card_size(
	void
)
{
	return SD_NUMBER_OF_BLOCKS;
}

int8_t
sd_spi_read_cid_register(
	sd_spi_cid_t *cid
)
{
	cid->mid = 0x03;
	memcpy(cid->oid, "SD", 2);
	memcpy(cid->pnm, "SU02G", 5);

	cid->prv_n = 0x8;
	cid->prv_m = 0x0;

	cid->psn_high = 0x07;
	cid->psn_mid_high = 0xD0;
	cid->psn_mid_low = 0xA2;
	cid->psn_low = 0xA8;

	cid->mdt_year = 0x0A;
	cid->mdt_month = 0x7;
	cid->crc = 0x7F;

	sd_spi_unselect_card();
	return SD_ERR_OK;
}

int8_t
sd_spi_read_csd_register(
	sd_spi_csd_t *csd
)
{
	csd->csd_structure = 0x1;
	csd->taac = 0x0E;
	csd->nsac = 0x00;
	csd->tran_speed = 0x032;
	csd->ccc_high = 0xF;
	csd->ccc_low = 0xFF;
	csd->max_read_bl_len = 0x0A;
	csd->read_bl_partial = 0x0;
	csd->write_bl_misalign = 0x0;
	csd->read_bl_misalign = 0x0;
	csd->dsr_imp = 0x0;

	csd->cvsi.v2.c_size_high = 0x00;
	csd->cvsi.v2.c_size_mid = 0x01;
	csd->cvsi.v2.c_size_low = 0xD8;

	csd->erase_bl_en = 0x1;
	csd->erase_sector_size = 0x7F;
	csd->wp_grp_size = 0x00;
	csd->wp_grp_enable = 0x0;
	csd->r2w_factor = 0x2;
	csd->write_bl_len = 0x9;
	csd->write_bl_partial = 0x0;
	csd->file_format_grp = 0x0;
	csd->copy = 0x0;
	csd->perm_write_protect = 0x0;
	csd->tmp_write_protect = 0x0;
	csd->file_format = 0x0;
	csd->crc = 0x7F;

	sd_spi_unselect_card();

	return SD_ERR_OK;
}

int8_t
sd_spi_card_status(
	void
)
{
	int8_t response = SD_ERR_OK;

	sd_spi_unselect_card();
	
	return response;
}

uint32_t
sd_spi_current_buffered_block(
	void
)
{
	return card.buffered_block_address;
}

/* For debugging purposes */
int8_t
print_page(
	uint32_t 	block_address
)
{
	if ((card.fp = fopen("data.raw", "rb+")) == NULL)
	{
		return SD_ERR_READ_FAILURE;
	}

	uint8_t buffer[512];

	fseek(card.fp, block_address << 9, SEEK_SET);
	fread(buffer, 512, 1, card.fp);

	if (fclose(card.fp) != 0)
	{
		return SD_ERR_READ_FAILURE;
	}

	printf("Page %d, Byte: %d\n", block_address, block_address * 512);

	uint16_t i;
	for (i = 0; i < 512; i++)
	{
		if (i % 16 == 0)
		{
			printf("\n");
		}

		printf("%d\t", buffer[i]);
	}

	printf("\n\n");
	return 0;
}

static void
sd_spi_clear_buffer(
	void
)
{
#if defined(SD_SPI_BUFFER)
	uint16_t i;
	for (i = 0; i < 512; i++) {
		card.sd_spi_buffer[i] = 0;
	}

	card.is_buffer_written = 0;
	card.is_buffer_current = 0;
#endif
}

static int8_t
sd_spi_write_out_data(
	uint32_t	block_address,
	void	 	*data,
	uint16_t 	number_of_bytes,
	uint16_t 	byte_offset
)
{
	sd_spi_select_card();

	if ((card.fp = fopen("data.raw", "rb+")) == NULL)
	{
		return SD_ERR_WRITE_FAILURE;
	}

	if (fseek(card.fp, block_address << 9, SEEK_SET) != 0)
	{
		return SD_ERR_WRITE_FAILURE;
	}

	uint8_t output_buffer[512];
	uint16_t i;

	/* Pad data with 0. */
	for (i = 0; i < byte_offset; i++)
	{
		output_buffer[i] = 0;
	}

	/* Write block. */
	for (i = byte_offset; i < byte_offset + number_of_bytes; i++)
	{
		output_buffer[i] = (((uint8_t*) data)[i - byte_offset]);
	}

	/* Pad data with 0. */
	for (i = byte_offset + number_of_bytes; i < 512; i++)
	{
		output_buffer[i] = 0;
	}

	if (card.is_read_write_continuous)
	{
		card.continuous_block_address++;

#if defined(SD_SPI_BUFFER)
		card.buffered_block_address = card.continuous_block_address;
#endif
	}

	fwrite(output_buffer, 512, 1, card.fp);
	
	if (fflush(card.fp) != 0)
	{
		return SD_ERR_WRITE_FAILURE;
	}

	if (fclose(card.fp) != 0)
	{
		return SD_ERR_WRITE_FAILURE;
	}

	num_writes++;
	return SD_ERR_OK;
}

static int8_t
sd_spi_read_in_data(
	uint32_t 	block_address,
	void	 	*data_buffer,
	uint16_t 	number_of_bytes,
	uint16_t 	byte_offset
)
{
	sd_spi_select_card();

	if ((card.fp = fopen("data.raw", "rb+")) == NULL)
	{
		return SD_ERR_READ_FAILURE;
	}

#if defined(SD_SPI_BUFFER)	/* Read block into sd_spi_buffer if it is defined. */
	fseek(card.fp, block_address << 9, SEEK_SET);
	fread(card.sd_spi_buffer, 512, 1, card.fp);

	if (card.is_read_write_continuous)
	{
		card.buffered_block_address = card.continuous_block_address;
		card.continuous_block_address++;
	}
	else
	{
		card.buffered_block_address = block_address;
	}

	card.is_buffer_current = 1;
#else
	fseek(card.fp, (block_address << 9) + byte_offset, SEEK_SET);
	fread(data_buffer, number_of_bytes, 1, card.fp);
#endif

	if (fclose(card.fp) != 0)
	{
		return SD_ERR_READ_FAILURE;
	}

	num_reads++;
	return SD_ERR_OK;
}

static void
sd_spi_select_card(
	void
)
{
#if defined(USE_HARDWARE_SPI)

#else
	if (card.is_chip_select_high)
	{
    	card.is_chip_select_high = 0;
	}
#endif
}

static void
sd_spi_unselect_card(
	void
)
{
#if defined(USE_HARDWARE_SPI)

#else
	if (!card.is_chip_select_high)
	{
    	card.is_chip_select_high = 1;
	}
#endif
}
