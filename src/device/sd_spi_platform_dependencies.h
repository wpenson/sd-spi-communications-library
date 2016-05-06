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