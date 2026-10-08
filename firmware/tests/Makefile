# Builds and runs the PC unit tests (host gcc, not the ARM compiler).
# Usage, from the firmware/tests folder:  make test

CC      ?= gcc
CFLAGS  := -std=c11 -Wall -Wextra -Werror -O0 -g -I../src
LDLIBS  := -lm

BUILD   := build

.PHONY: test clean

test: $(BUILD)/test_thermistor
	./$(BUILD)/test_thermistor

$(BUILD)/test_thermistor: test_thermistor.c ../src/thermistor.c ../src/thermistor.h | $(BUILD)
	$(CC) $(CFLAGS) test_thermistor.c ../src/thermistor.c -o $@ $(LDLIBS)

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)
