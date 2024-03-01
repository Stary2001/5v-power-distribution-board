#pragma once
#include <stdio.h>
#include <stdarg.h>

#include "W5500/W5500.hpp"
#include "W5500/Protocols/DHCP.hpp"
#include "SamdBus.hpp"

template<int N> class EtherBus : public W5500::Buses::SAMDBus<N> {
    // Inherit constructor
    using W5500::Buses::SAMDBus<N>::SAMDBus;

    virtual void init() {
        // Super init
        W5500::Buses::SAMDBus<N>::init();

        // Initialize LFSR (for PRNG) using unique ID of this chip
        /*uint8_t mac_addr[6];
        esp_err_t ret = esp_efuse_mac_get_default(mac_addr);
        ESP_ERROR_CHECK(ret);
        memset(&_lfsr_state, 0, 8);
        memcpy(&_lfsr_state, mac_addr, 6);*/
        // todo lol
    }

    virtual uint64_t millis() override {
        return 2; //esp_timer_get_time() / 1000;
    }

    // Proxy logging from network stack to uart.
    // Again, you need to provide your own implementation here.
    virtual void log(const char *msg, ...) override {
        va_list args;
        va_start(args, msg);
        vprintf(msg, args);
        va_end(args);
    }
};
