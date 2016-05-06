#include "sd_spi.h"
#include "planck_unit/src/planckunit.h"

#define CHIP_SELECT_PIN 4
uint8_t data[512];

static void
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

static void
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

void
test_sd_spi_initialization(
	planck_unit_test_t *tc
)
{
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_init(CHIP_SELECT_PIN));
}

void
test_sd_spi_single_write_and_read(
	planck_unit_test_t *tc
)
{
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_init(CHIP_SELECT_PIN));
	populate_data_array_1();

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_write(1, data, 512, 0));
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_write(1, data, 10, 0));
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_flush());

	uint8_t buffer[11];
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_read(1, buffer, 11, 0));

	uint8_t i;
	for (i = 0; i < 10; i++)
	{
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, data[i], buffer[i]);
	}

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 'k', buffer[10]);
}

void
test_sd_spi_single_block_write_and_read(
	planck_unit_test_t *tc
)
{
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_init(CHIP_SELECT_PIN));
	populate_data_array_1();
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_write_block(1, data));

	uint8_t buffer[27];
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_read(1, buffer, 27, 2));

	uint8_t i;
	for (i = 0; i < 27; i++)
	{
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, data[i + 2], buffer[i]);
	}
}

void
test_sd_spi_continuous_block_write(
	planck_unit_test_t *tc
)
{
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_init(CHIP_SELECT_PIN));
	populate_data_array_1();
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_write_continuous_start(100, 100));

	uint8_t i;
	for (i = 0; i < 10; i++)
	{
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_write_continuous(data, 512, 0));
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_write_continuous_next());
	}

	/* Write only 10 bytes out to a block. The rest of the block should contain
	   only 0's */
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_write_continuous(data, 10, 0));
	/* Don't need to call write flush since sd_spi_write_continuous_stop should
	   handle it */
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_write_continuous_stop());

	uint8_t buffer[27];

	for (i = 100; i < 110; i++)
	{
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_read(i, buffer, 27, 2));

		uint8_t j;
		for (j = 0; j < 27; j++)
		{
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, data[j + 2], buffer[j]);
		}
	}

	/* Test if rest of data in block is 0 */
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_read(110, buffer, 11, 2));
	PLANCK_UNIT_ASSERT_TRUE(tc, buffer[10] == 0x00);
}

void
test_sd_spi_continuous_block_read(
	planck_unit_test_t *tc
)
{
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_init(CHIP_SELECT_PIN));
	populate_data_array_1();

	uint8_t i;
	for (i = 0; i < 10; i++)
	{
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_write_block(i, data));
	}

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_read_continuous_start(0));

	uint8_t buffer[27];
	for (i = 0; i < 10; i++)
	{
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_read_continuous(buffer, 27, 2));

		uint8_t j;
		for (j = 0; j < 27; j++)
		{
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, data[j + 2], buffer[j]);
		}

		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_read_continuous_next());
	}

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_read_continuous_stop());
}

void
test_sd_spi_multiple_reads_and_writes(
	planck_unit_test_t *tc
)
{
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_init(CHIP_SELECT_PIN));

	populate_data_array_1();
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_write(200, data, 512, 0));
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_flush());

	populate_data_array_2();
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_write(201, data, 512, 0));
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_flush());

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_read_continuous_start(200));

	uint8_t buffer[27];

	uint8_t i;
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_read_continuous(buffer, 27, 2));
	populate_data_array_1();

	for (i = 0; i < 27; i++)
	{
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, data[i + 2], buffer[i]);
	}

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_read_continuous_next());
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_read_continuous(buffer, 27, 2));
	populate_data_array_2();

	for (i = 0; i < 27; i++)
	{
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, data[i + 2], buffer[i]);
	}

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_read_continuous_stop());

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_write_continuous_start(200, 100));

	for (i = 0; i < 10; i++)
	{
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_write_continuous(data, 512, 0));
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_write_continuous_next());
	}

	/* Write only 10 bytes out to a block. The rest of the block should contain
	   only 0's */
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_write_continuous(data, 10, 0));
	/* Don't need to call write flush since sd_spi_write_continuous_stop should
	   handle it */
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_write_continuous_stop());

	for (i = 200; i < 210; i++)
	{
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_read(i, buffer, 27, 2));

		uint8_t j;
		for (j = 0; j < 27; j++)
		{
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, data[j + 2], buffer[j]);
		}
	}

	/* Test if rest of data in block is 0 */
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_read(210, buffer, 11, 0));
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0x00, buffer[10]);
}

void
test_sd_spi_erase_blocks(
	planck_unit_test_t *tc
)
{
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_init(CHIP_SELECT_PIN));
	populate_data_array_1();
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_write_block(1, data));
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_write_block(2, data));

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_erase_blocks(1, 2));

	uint8_t val;
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, sd_spi_read(1, &val, 1, 2));
	PLANCK_UNIT_ASSERT_TRUE(tc, val == 0x00 || val == 0xFF);
}

planck_unit_suite_t*
tefs_getsuite(
	void
)
{
	planck_unit_suite_t *suite = planck_unit_new_suite();

	planck_unit_add_to_suite(suite, test_sd_spi_initialization);
	planck_unit_add_to_suite(suite, test_sd_spi_single_write_and_read);
	planck_unit_add_to_suite(suite, test_sd_spi_single_block_write_and_read);
	planck_unit_add_to_suite(suite, test_sd_spi_continuous_block_write);
	planck_unit_add_to_suite(suite, test_sd_spi_continuous_block_read);
	planck_unit_add_to_suite(suite, test_sd_spi_multiple_reads_and_writes);
	planck_unit_add_to_suite(suite, test_sd_spi_erase_blocks);

	return suite;
}

void
runalltests_sd_spi(
	void
)
{
	planck_unit_suite_t	*suite	= tefs_getsuite();
	planck_unit_run_suite(suite);
	planck_unit_destroy_suite(suite);
}
