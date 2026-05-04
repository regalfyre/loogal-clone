CC ?= gcc
CFLAGS ?= -O3 -Wall -Wextra -std=c11 -Iinclude
LDFLAGS ?=
SRC := $(wildcard src/*.c)
OBJ := $(SRC:.c=.o)
BIN := loogal

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

clean:
	rm -f src/*.o $(BIN)

safe-clean: clean
	@echo "Preserving data/, logs/, manifests/"

test: $(BIN)
	bash tests/smoke.sh

.PHONY: all clean safe-clean test
