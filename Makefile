MMCU=atmega328p
F_CPU=12000000UL
CFLAGS=-Wall -mmcu=${MMCU} -DF_CPU=${F_CPU} -Os

SRC_DIR = src
BUILD_DIR = build

# use a different port depending on OS
PORT=/dev/tty.usbmodem*
ifeq ($(OS), Windows_NT)
	PORT=COM5
endif

flash: build
	avrdude -p ${MMCU} -c arduino -P ${PORT} -vv -U flash:w:main.hex

build: main.hex

%.hex: $(BUILD_DIR)/%.obj
	avr-objcopy -O ihex $< $@

$(BUILD_DIR)/%.obj: $(SRC_DIR)/%.c
	avr-gcc ${CFLAGS} $< -o $@
clean:
	rm $(BUILD_DIR)/*.obj *.hex *.o

.PHONY: flash build clean
.DEFAULT_GOAL := build
