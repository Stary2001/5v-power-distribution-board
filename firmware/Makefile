CROSS =--cross-file=cross-arm.txt --cross-file=cross-samd21.txt

all: build/firmware.elf build/firmware.bin

build/build.ninja:
	meson setup build $(CROSS) -Db_lto=true

.PHONY: build/firmware.elf dfu clean oocd flash gdb
build/firmware.elf: build/build.ninja
	ninja -C build

build/firmware.bin: build/firmware.elf
	arm-none-eabi-objcopy -O binary build/firmware.elf build/firmware.bin

build/firmware.dfu: build/firmware.elf
	dx1elf2dfu build/firmware.elf build/firmware.dfu

dfu: build/firmware.dfu
	dfu-util -d 1209:2003 -D build/firmware.dfu

clean:
	ninja -C build clean

oocd:
	openocd -f interface/cmsis-dap.cfg -c "cmsis_dap_vid_pid 0x1a86 0x8012" -f target/at91samdXX.cfg

flash: build/firmware.bin
	openocd -f interface/cmsis-dap.cfg -c "cmsis_dap_vid_pid 0x1a86 0x8012" -f target/at91samdXX.cfg -c "program build/firmware.bin; reset; exit"

gdb:
	arm-none-eabi-gdb -ex "target extended-remote localhost:3333" build/firmware.elf
