# Makefile for your Atmega2560 project

MCU=atmega2560
PROGRAMMER=wiring
PARTNO=m2560

PORT=/dev/ttyACM0  # Change this to your port

# Define the source files
SRC = $(wildcard *.c) $(wildcard */*.c) $(wildcard progs/*/*.c)
HEADERS = -I /usr/lib/avr/include/ -I./ -I./lib -I./progs -I./progs/tests -I./progs/user_programs

# Define the object files
OBJ = $(SRC:.c=.o)

# Define the compiler and linker
CC = avr-gcc
CFLAGS = -c -mmcu=$(MCU) -Wall -g -Og -fno-jump-tables
LDFLAGS = -mmcu=$(MCU) -Wl,--gc-sections

# Define the final executable
TARGET = main.hex

SILENT = @
ECHO = echo

all: $(TARGET) flash

#.PHONY: main
#main:
#	echo "Compiling the following files:"
#	ls *.c */*.c progs/*/*.c
#	avr-gcc -mmcu=$(MCU) -Wall -g -Og -o main.o -I./ -I./lib -I./progs -I./progs/tests -I./progs/user_programs *.c */*.c progs/*/*.c
#	avr-objcopy -j .text -j .data -O ihex main.o main.hex

# Rule for compiling source files into object files
%.o: %.c
	$(SILENT) $(ECHO) "Compiling $<..."
	$(SILENT) $(CC) $(CFLAGS) $(HEADERS) -c $< -o $@

# Rule for linking object files into the final executable
$(TARGET): $(OBJ)
	$(SILENT) $(ECHO) "Linking object files..."
	$(SILENT) $(CC) $(LDFLAGS) $(OBJ) -o main.elf
	$(SILENT) $(ECHO) "Creating HEX file..."
	$(SILENT) avr-objcopy -j .text -j .data -O ihex main.elf $(TARGET)

# Define the main target
#main: $(TARGET)

# Flash the target to the board
.PHONY: flash
flash:
	$(SILENT) avrdude -v -p $(PARTNO) -c $(PROGRAMMER) -U flash:w:$(TARGET):i -F -P $(PORT) -D

# Clean up build files
.PHONY: clean
clean:
	$(SILENT) $(ECHO) "Cleaning up..."
	$(SILENT) find . -type f \( -name "*.o" -o -name "*.hex" \) -exec rm -f {} +

