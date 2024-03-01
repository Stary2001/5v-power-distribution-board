#ifndef _W5500__W5500_BUSES_SAMDBUS_H_
#define _W5500__W5500_BUSES_SAMDBUS_H_

#include <stdint.h>

#include "sam/sercom_spi.h"
#include "sam/gpio.h"

#include <W5500/Bus.hpp>

namespace W5500 {
namespace Buses {

template<int N> class SAMDBus: public Bus {
  public:
    SAMDBus(uint32_t spi_dipo, uint32_t spi_dopo, uint32_t cs_port, uint32_t cs_pin)
        : _spi_dipo(spi_dipo), _spi_dopo(spi_dopo), _cs_port(cs_port), _cs_pin(cs_pin) {}
    ~SAMDBus() override{};

    void init() override {
        spi.init(_spi_dipo, _spi_dopo);
        Bus::init();

        port_set_direction(_cs_port, _cs_pin, true);
        //gpio_mode_setup(_cs_port, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, _cs_pin);
        chip_deselect();
    };

    // SPI read/write method
    void spi_xfer(uint8_t send, uint8_t *recv) override {
        spi.send_byte(send);
        *recv = spi.read_byte();
    }

    // Chip select pin manipulation
    void chip_select() override { port_set_value(_cs_port, _cs_pin, false); }
    virtual void chip_deselect() override { port_set_value(_cs_port, _cs_pin, true); }

  private:
    SercomSPI<N> spi;
    const uint32_t _spi_dipo;
    const uint32_t _spi_dopo;
    const uint32_t _cs_port;
    const uint32_t _cs_pin;
};
} // namespace Buses
} // namespace W5500

#endif // #ifndef _W5500__W5500_BUSES_OPENCM3_H_
