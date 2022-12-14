MMCU=atmega328p
F_CPU=12000000UL
CFLAGS=-g -Wall -mmcu=${MMCU} -DF_CPU=${F_CPU} -Os -mcall-prologues -std=c99
# CFLAGS=-g -Wall -mmcu=${MMCU} -DF_CPU=${F_CPU} -Os -mcall-prologues -DDEBUG -std=c99

TARGET = main
SRC = $(wildcard src/*.c) $(wildcard src/controller/*.c) $(wildcard src/drivers/*.c) $(wildcard src/interfaces/*.c) $(wildcard src/graphics/*.c) 

OBJ = $(SRC:%.c=%.o)

# use a different port depending on OS
PORT=/dev/tty.usbmodem*
ifeq ($(OS), Windows_NT)
	PORT=COM5
endif

flash: build
	avrdude -p ${MMCU} -c arduino -P $(PORT) -vv -U flash:w:$(TARGET).hex

build: $(TARGET).hex size

%.hex: $(TARGET).elf
	avr-objcopy -O ihex $< $@

%.elf: $(OBJ)
	avr-gcc ${CFLAGS} $^ -o $@

%.o: %.c
	avr-gcc -c ${CFLAGS} $< -o $@

clean:
	rm -f $(TARGET).elf $(OBJ) $(TARGET).hex 

size: $(TARGET).elf
	avr-size --format=avr --mcu=$(MMCU) $^

size-o:$(OBJ)
	avr-size --format=berkeley --mcu=$(MMCU) -d $^

.PHONY: flash build clean size
.DEFAULT_GOAL := build
