#include "mongoose.h"
#include "sam/sercom_spi.h"


#include "sam/gpio.h"
#include "sam/pinmux.h"
#include "sam/serial_number.h"
#include "mongoose_integration.h"


// http://9net.org/screenshots/1709135387.png
// ethernet

const int ETH_MOSI = 16;
const int ETH_CLK = 17;
const int ETH_CS = 18;
const int ETH_MISO = 19;
const int ETH_INT = 21;
const int ETH_RST = 21;

SercomSPI<3> sercom_spi;
struct mg_mgr mgr;  // Mongoose event manager

struct mg_tcpip_spi spi = {
	NULL,                                               // SPI data
	[](void *) { port_set_value(PORT_A, ETH_CS, false); },          // begin transation
	[](void *) { port_set_value(PORT_A, ETH_CS, true); },         // end transaction
	[](void * data, uint8_t c) { sercom_spi.send_byte(c); return sercom_spi.read_byte(); },  // execute transaction
};

struct mg_tcpip_if mif = {
	.mac = {2, 0, 1, 2, 3, 5},
	.driver = &mg_tcpip_driver_w5500,
	.driver_data = &spi
};  // network interface

const char *index_html = "<!doctype html>"
"<html>"
"<head>"
"<title>Power</title>"
"<style> #control { max-width: 30%; display: grid;  grid-template-columns: 1fr 1fr 1fr 1fr; grid-template-rows: 1fr 1fr;} .port { border: 1px black;} .port-stats { } </style>"
"</head>"
"<body>"
"<div id='control'>"
"<div class='port'><div class='port-stats'>Stats stats</div><button onclick='toggle(1)'>1</button></div>"
"<div class='port'><div class='port-stats'>Stats stats</div><button onclick='toggle(2)'>2</button></div>"
"<div class='port'><div class='port-stats'>Stats stats</div><button onclick='toggle(3)'>3</button></div>"
"<div class='port'><div class='port-stats'>Stats stats</div><button onclick='toggle(4)'>4</button></div>"
"<div class='port'><button onclick='toggle(5)'>5</button><div class='port-stats'>Stats stats</div></div>"
"<div class='port'><button onclick='toggle(6)'>6</button><div class='port-stats'>Stats stats</div></div>"
"<div class='port'><button onclick='toggle(7)'>7</button><div class='port-stats'>Stats stats</div></div>"
"<div class='port'><button onclick='toggle(8)'>8</button><div class='port-stats'>Stats stats</div></div>"
"</div>"
"</body>"
"<script>"
"function toggle(i) { ws.send('toggle ' + i.toString()); }"
"var ws;"
"ws = new WebSocket('ws://' + location.host + '/websocket');"
"ws.onopen = function() { ws.send('ping'); };"
"ws.onmessage = function(ev) { console.log(ev.data); }"
"</script>"
"</html>";

struct mg_connection *websocket_conn = nullptr;

static void http_event_callback(struct mg_connection *c, int ev, void *ev_data) {
  if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    if (mg_http_match_uri(hm, "/websocket")) {
		websocket_conn = c;
    	mg_ws_upgrade(c, hm, NULL);
    } else if (mg_http_match_uri(hm, "/") || mg_http_match_uri(hm, "/index.html")) {
	  mg_http_reply(c, 200, "", "%s", index_html);
    } else {
		mg_http_reply(c, 404, "", "Not found!");
	}
  } else if (ev == MG_EV_WS_MSG) {
    struct mg_ws_message *wm = (struct mg_ws_message *) ev_data;
    mg_ws_send(c, "ok", 2, WEBSOCKET_OP_TEXT);
	printf("websocket message: '%.*s'\r\n", wm->data.len, wm->data.ptr);
  } else if(ev == MG_EV_WS_CTL) {
	struct mg_ws_message *wm = (struct mg_ws_message *) ev_data;
	uint8_t op = wm->flags & 15;
	if(op == WEBSOCKET_OP_CLOSE && c == websocket_conn) {
		printf("websocket closed\r\n");
		websocket_conn = nullptr;
	}
  }
}

int counter = 0;
void ethernet_init() {
	port_set_value(PORT_A, ETH_CS, true);
	port_set_direction(PORT_A, ETH_CS, true);

	port_set_direction(PORT_A, ETH_INT, false);
	port_set_direction(PORT_A, ETH_RST, true);
	port_set_value(PORT_A, ETH_RST, true);

	port_set_function(PORT_A, ETH_CLK, 3); // sercom alt, sercom 3
	port_set_function(PORT_A, ETH_MISO, 3);
	port_set_function(PORT_A, ETH_MOSI, 3);
	port_set_pmux_enable(PORT_A, ETH_CLK, true);
	port_set_pmux_enable(PORT_A, ETH_MISO, true);
	port_set_pmux_enable(PORT_A, ETH_MOSI, true);

	port_set_value(PORT_A, ETH_RST, false);
	// sleep
	for(volatile int i = 0; i < 1000000; i++) {}
	port_set_value(PORT_A, ETH_RST, true);
	// sleep
	for(volatile int i = 0; i < 1000000; i++) {}

	sercom_spi.init(3, 0, 1000000);
	mg_mgr_init(&mgr);
	mg_log_set(MG_LL_DEBUG);
	mg_tcpip_init(&mgr, &mif);

	// Start a 5 sec timer, print status message periodically
	mg_timer_add(
		&mgr, 5000, MG_TIMER_REPEAT,
		[](void *) {
			if(websocket_conn != nullptr) {
				char buff[64]; 
				size_t len = sprintf(buff, "ping %i\n", counter++);
				mg_ws_send(websocket_conn, buff, len, WEBSOCKET_OP_TEXT);
			} else {
				printf("No websocket!\r\n");
			}
			MG_INFO(("ethernet: %s", mg_tcpip_driver_w5500.up(&mif) ? "up" : "down"));
		},
		NULL);

    mg_http_listen(&mgr, "http://0.0.0.0", http_event_callback, &mgr);
}

void ethernet_tick() {
    mg_mgr_poll(&mgr, 1);
}
