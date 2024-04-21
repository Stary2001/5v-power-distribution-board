#include <stdio.h>
#include "sam/heap.h"
#include "sam/gpio.h"
#include "sam/bod.h"
#include "sam/clock.h"
#include "sam/sercom_usart.h"
#include "sam/pinmux.h"
#include "sam/serial_number.h"
#include "sam/adc.h"
#include "mongoose_integration.h"

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

extern uint64_t __gtod_millis;
uint64_t system_ticks = 0;
bool value = true;
extern "C" void SysTick_Handler (void)
{
	system_ticks++;
	__gtod_millis = system_ticks;
}

char fake_heap[4096 * 3 - 800];

int main() {
	__init_heap(fake_heap, sizeof(fake_heap));

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

	ethernet_init();

	uint32_t last_system_tick = 0;
	while(true) {
		ethernet_tick();

		if((system_ticks % 100) == 0 && system_ticks != last_system_tick) {
			last_system_tick = system_ticks;
		}
	}

	return 0;
}