MMCU=atmega328p
F_CPU=12000000UL
CFLAGS=-g -Wall -mmcu=${MMCU} -DF_CPU=${F_CPU} -Os -mcall-prologues -DDEBUG

TARGET = main
SRC = src/$(TARGET).c src/logging.c
OBJ = $(SRC:%.c=%.o)


flash: build
	avrdude -p ${MMCU} -c arduino -P /dev/tty.usbmodem* -vv -U flash:w:$(TARGET).hex

build: $(TARGET).hex size

%.hex: $(TARGET).elf
	avr-objcopy -O ihex $< $@

%.elf: $(OBJ)
	avr-gcc ${CFLAGS} $^ -o $@

%.o: %.c
	avr-gcc -c ${CFLAGS} $< -o $@

clean:
	rm *.elf *.obj *.hex *.o

size: $(TARGET).elf
	avr-size --format=avr --mcu=$(MMCU) $^

.PHONY: flash build clean size
.DEFAULT_GOAL := build
