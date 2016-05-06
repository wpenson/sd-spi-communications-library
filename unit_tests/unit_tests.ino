#include <SPI.h>
#include "sd_spi_tests.c"

void
setup(
)
{
	Serial.begin(115200);

	while (!Serial) {
    	; // wait for serial port to connect. Needed for Leonardo only
 	}

 	runalltests_tefs();
}

void
loop(
)
{

}
