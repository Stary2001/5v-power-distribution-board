#include <stdio.h>
#include "sam/gpio.h"
#include "sam/bod.h"
#include "sam/clock.h"
#include "sam/sercom_usart.h"
#include "sam/sercom_spi.h"
#include "sam/pinmux.h"
#include "sam/serial_number.h"
#include "sam/adc.h"

#include "mongoose.h"

/* pa00-01 are i2c */
/* pa02 - pa08, pb08,09 are isense */
/* pb03 is vsense */
/* pa8-15 are the switches */
/* pa16-pa21 are ethernet spi+int/rst */
/* pa22 is led3 */
/* pa24,25 are usb ofc */

const unsigned int PORT1_ISENSE_A = 2;
const unsigned int PORT1_ISENSE_B = 3;
const unsigned int PORT2_ISENSE_A = 8; // port b
const unsigned int PORT2_ISENSE_B = 9; // port b
const unsigned int PORT3_ISENSE_A = 6;
const unsigned int PORT3_ISENSE_B = 7;
const unsigned int PORT4_ISENSE_A = 4;
const unsigned int PORT4_ISENSE_B = 5;

const unsigned int PORT1_ENABLE_A = 8;
const unsigned int PORT1_ENABLE_B = 9;
const unsigned int PORT2_ENABLE_A = 10;
const unsigned int PORT2_ENABLE_B = 11;
const unsigned int PORT3_ENABLE_A = 12;
const unsigned int PORT3_ENABLE_B = 13;
const unsigned int PORT4_ENABLE_A = 14;
const unsigned int PORT4_ENABLE_B = 15;


const unsigned int LED1 = 10; // port b
const unsigned int LED2 = 11; // port b
const unsigned int LED3 = 22; // port a
const unsigned int LED4 = 23; // porta

const unsigned int port_gpios[] = {
	PORT1_ENABLE_A,
	PORT1_ENABLE_B,
	PORT2_ENABLE_A,
	PORT2_ENABLE_B,
	PORT3_ENABLE_A,
	PORT3_ENABLE_B,
	PORT4_ENABLE_A,
	PORT4_ENABLE_B
};

struct port { unsigned int port; unsigned int num; };
const struct port port_adcs[] = {
	{PORT_A, PORT1_ISENSE_A},
	{PORT_A, PORT1_ISENSE_B},
	{PORT_B, PORT2_ISENSE_A},
	{PORT_B, PORT2_ISENSE_B},
	{PORT_B, PORT3_ISENSE_A},
	{PORT_B, PORT3_ISENSE_B},
	{PORT_B, PORT4_ISENSE_A},
	{PORT_B, PORT4_ISENSE_B},
};

volatile uint32_t system_ticks = 0;
bool value = true;
extern "C" void SysTick_Handler (void)
{
	system_ticks++;
	if((system_ticks % 5000) == 0) {
		port_set_value(PORT_A, LED3, value);
		//port_set_value(PORT_A, PORT1_ENABLE_A, value);
		//port_set_value(PORT_A, PORT1_ENABLE_B, value);
		value = !value;
	}
}

 

int main() {
	clock_switch_to_8mhz();
	bod_init();
	bod_set_3v3();

	clock_switch_to_48mhz_from_usb();
	clock_setup_gclk2_8mhz();

	clock_setup_systick_1ms();

	for(unsigned int i = 0; i < 8; i++) {
		port_set_value(PORT_A, port_gpios[i], false);
		port_set_direction(PORT_A, port_gpios[i], true);

		port_set_function(port_adcs[i].port, port_adcs[i].num, 1);
		port_set_pmux_enable(port_adcs[i].port, port_adcs[i].num, true);
	}

	port_set_direction(PORT_B, LED1, true);
	port_set_value(PORT_B, LED1, true);

	port_set_direction(PORT_B, LED2, true);
	port_set_value(PORT_B, LED2, true);

	port_set_direction(PORT_A, LED3, true);
	port_set_value(PORT_A, LED3, true);

	port_set_direction(PORT_A, LED4, true);
	port_set_value(PORT_A, LED4, true);


	// sercom setup
	port_set_function(PORT_A, 0, 3);
	port_set_function(PORT_A, 1, 3);
	port_set_pmux_enable(PORT_A, 0, true);
	port_set_pmux_enable(PORT_A, 1, true);

	// enable port1 a
	port_set_value(PORT_A, PORT1_ENABLE_A, true);
	port_set_value(PORT_A, PORT1_ENABLE_B, true);

	SercomUart<1> sercom;
	sercom.init(0, 1); // sercom[0] used for tx, sercom[1] used for rx

	ADCClass adc;
	adc.init(4, 1);
	adc.select(0);

	// http://9net.org/screenshots/1709135387.png
	// ethernet

	const int ETH_MOSI = 16;
	const int ETH_CLK = 17;
	const int ETH_CS = 18;
	const int ETH_MISO = 19;
	const int ETH_INT = 21;
	const int ETH_RST = 21;


	port_set_value(ETH_CS, true);
	port_set_direction(ETH_CS, true);

	port_set_direction(PORT_A, ETH_INT, false);
	port_set_direction(PORT_A, ETH_RST, true);
	port_set_value(PORT_A, ETH_RST, true);

	port_set_function(PORT_A, ETH_CLK, 3); // sercom alt, sercom 3
	port_set_function(PORT_A, ETH_MISO, 3);
	port_set_function(PORT_A, ETH_MOSI, 3);
	port_set_pmux_enable(PORT_A, ETH_CLK, true);
	port_set_pmux_enable(PORT_A, ETH_MISO, true);
	port_set_pmux_enable(PORT_A, ETH_MOSI, true);

	SercomSPI<3> sercom;

	struct mg_tcpip_spi spi = {
		NULL,                                               // SPI data
		[](void *) { port_set_value(PORT_A, ETH_CS, false); },          // begin transation
		[](void *) { port_set_value(PORT_A, ETH_CS, true); },         // end transaction
		[](void *, uint8_t c) {  },  // execute transaction
	};

	struct mg_mgr mgr;  // Mongoose event manager
	struct mg_tcpip_if mif = {.mac = {2, 0, 1, 2, 3, 5}};  // network interface
    mg_mgr_init(&mgr);

	mif.driver = &mg_tcpip_driver_w5500;
	mif.driver_data = &spi;
	mg_tcpip_init(&mgr, &mif);

	uint32_t last_system_tick = 0;
	bool led = false;
	while(true) {
		if((system_ticks % 100) == 0 && system_ticks != last_system_tick) {
			last_system_tick = system_ticks;

			uint16_t values[8] = {0};
			adc.select(0);
			adc.read_with_input_scan(values, 8);
			printf("\033[H");
			char buff[128];
			for(int i = 0; i < 8; i++) { 
				printf("adc readout %i: %i        \r\n", i, values[i]);
			}

			adc.select(11);
			uint16_t vbus = adc.read();
			printf(buff, "vbus adc readout %i         \r\n", vbus);

			port_set_value(PORT_A, LED4, led);
			led=!led;
		}
	}

	return 0;
}
