#include <stdio.h>
#include "sam/gpio.h"
#include "sam/bod.h"
#include "sam/clock.h"
#include "sam/sercom_usart.h"
#include "sam/pinmux.h"
#include "sam/serial_number.h"

#include "tusb.h"

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
const unsigned int LED3 = 22;

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
		port_set_value(PORT_A, PORT1_ENABLE_A, value);
		port_set_value(PORT_A, PORT1_ENABLE_B, value);
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

	while(true) { asm(""); }
	return 0;
}
