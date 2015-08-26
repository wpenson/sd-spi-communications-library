#line 2 "sd_spi_test.ino"
#include <ArduinoUnit.h>

#include <SPI.h>
#include "sd_spi.h"

#define CHIP_SELECT_PIN 4
uint8_t data[512];

void
populate_data_array_1(
	void
)
{
	long int i;

	for (i = 0; i < 26; i++) {
		data[i] = 'a' + i;
	}

	for (i = 26; i < 512; i++) {
		data[i] = '.';
	}
}

void
populate_data_array_2(
	void
)
{
	long int i;

	for (i = 0; i < 26; i++) {
		data[i] = 'A' + i;
	}

	for (i = 26; i < 512; i++) {
		data[i] = '!';
	}
}

test(
	initialization
)
{
	assertEqual(0, sd_spi_init(CHIP_SELECT_PIN));
}

test(
	single_write_and_read
)
{
	assertEqual(0, sd_spi_init(CHIP_SELECT_PIN));
	populate_data_array_1();

	assertEqual(0, sd_spi_write(1, data, 512, 0));
	assertEqual(0, sd_spi_write(1, data, 10, 0));
	assertEqual(0, sd_spi_flush());

	uint8_t buffer[11];
	assertEqual(0, sd_spi_read(1, buffer, 11, 0));

	uint8_t i;
	for (i = 0; i < 10; i++)
	{
		assertEqual(data[i], buffer[i]);
	}

	assertEqual('k', buffer[10]);
}

test(
	single_block_write_and_read
)
{
	assertEqual(0, sd_spi_init(CHIP_SELECT_PIN));
	populate_data_array_1();
	assertEqual(0, sd_spi_write_block(1, data));

	uint8_t buffer[27];
	assertEqual(0, sd_spi_read(1, buffer, 27, 2));

	uint8_t i;
	for (i = 0; i < 27; i++)
	{
		assertEqual(data[i + 2], buffer[i]);
	}
}

test(
	continuous_block_write
)
{
	assertEqual(0, sd_spi_init(CHIP_SELECT_PIN));
	populate_data_array_1();
	assertEqual(0, sd_spi_write_continuous_start(100, 100));

	uint8_t i;
	for (i = 0; i < 10; i++)
	{
		assertEqual(0, sd_spi_write_continuous(data, 512, 0));
		assertEqual(0, sd_spi_write_continuous_next());
	}

	/* Write only 10 bytes out to a block. The rest of the block should contain
	   only 0's */
	assertEqual(0, sd_spi_write_continuous(data, 10, 0));
	/* Don't need to call write flush since sd_spi_write_continuous_stop should
	   handle it */
	assertEqual(0, sd_spi_write_continuous_stop());

	uint8_t buffer[27];

	for (i = 100; i < 110; i++)
	{
		assertEqual(0, sd_spi_read(i, buffer, 27, 2));

		uint8_t j;
		for (j = 0; j < 27; j++)
		{
			assertEqual(data[j + 2], buffer[j]);
		}
	}

	/* Test if rest of data in block is 0 */
	assertEqual(0, sd_spi_read(110, buffer, 11, 2));
	assertTrue(buffer[10] == 0x00);
}

test(
	continuous_block_read
)
{
	assertEqual(0, sd_spi_init(CHIP_SELECT_PIN));
	populate_data_array_1();

	uint8_t i;
	for (i = 0; i < 10; i++)
	{
		assertEqual(0, sd_spi_write_block(i, data));
	}

	assertEqual(0, sd_spi_read_continuous_start(0));

	uint8_t buffer[27];
	for (i = 0; i < 10; i++)
	{
		assertEqual(0, sd_spi_read_continuous(buffer, 27, 2));

		uint8_t j;
		for (j = 0; j < 27; j++)
		{
			assertEqual(data[j + 2], buffer[j]);
		}

		assertEqual(0, sd_spi_read_continuous_next());
	}

	assertEqual(0, sd_spi_read_continuous_stop());
}

test(
	multiple_reads_and_writes
)
{
	assertEqual(0, sd_spi_init(CHIP_SELECT_PIN));

	populate_data_array_1();
	assertEqual(0, sd_spi_write(200, data, 512, 0));
	assertEqual(0, sd_spi_flush());

	populate_data_array_2();
	assertEqual(0, sd_spi_write(201, data, 512, 0));
	assertEqual(0, sd_spi_flush());

	assertEqual(0, sd_spi_read_continuous_start(200));

	uint8_t buffer[27];

	uint8_t i;
	assertEqual(0, sd_spi_read_continuous(buffer, 27, 2));
	populate_data_array_1();

	for (i = 0; i < 27; i++)
	{
		assertEqual(data[i + 2], buffer[i]);
	}

	assertEqual(0, sd_spi_read_continuous_next());
	assertEqual(0, sd_spi_read_continuous(buffer, 27, 2));
	populate_data_array_2();

	for (i = 0; i < 27; i++)
	{
		assertEqual(data[i + 2], buffer[i]);
	}

	assertEqual(0, sd_spi_read_continuous_stop());

	assertEqual(0, sd_spi_write_continuous_start(200, 100));

	for (i = 0; i < 10; i++)
	{
		assertEqual(0, sd_spi_write_continuous(data, 512, 0));
		assertEqual(0, sd_spi_write_continuous_next());
	}

	/* Write only 10 bytes out to a block. The rest of the block should contain
	   only 0's */
	assertEqual(0, sd_spi_write_continuous(data, 10, 0));
	/* Don't need to call write flush since sd_spi_write_continuous_stop should
	   handle it */
	assertEqual(0, sd_spi_write_continuous_stop());

	for (i = 200; i < 210; i++)
	{
		assertEqual(0, sd_spi_read(i, buffer, 27, 2));

		uint8_t j;
		for (j = 0; j < 27; j++)
		{
			assertEqual(data[j + 2], buffer[j]);
		}
	}

	/* Test if rest of data in block is 0 */
	assertEqual(0, sd_spi_read(210, buffer, 11, 0));
	assertEqual(0x00, buffer[10]);
}

test(
	erase_blocks
)
{
	assertEqual(0, sd_spi_init(CHIP_SELECT_PIN));
	populate_data_array_1();
	assertEqual(0, sd_spi_write_block(1, data));
	assertEqual(0, sd_spi_write_block(2, data));

	assertEqual(0, sd_spi_erase_blocks(1, 2));

	uint8_t val;
	assertEqual(0, sd_spi_read(1, &val, 1, 2));
	assertTrue(val == 0x00 || val == 0xFF);
}

void
setup(
)
{
	Serial.begin(115200);

	while (!Serial) {
    	; // wait for serial port to connect. Needed for Leonardo only
 	}
}

void
loop(
)
{
	Test::run();
}
