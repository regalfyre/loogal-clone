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

test-search-json: $(BIN)
	bash tests/test_search_json_build.sh


test-action: $(BIN)
	bash tests/test_action_module.sh

.PHONY: test-action


test-session: $(BIN)
	bash tests/test_session_module.sh

.PHONY: test-session


test-history: $(BIN)
	bash tests/test_history_module.sh

.PHONY: test-history


test-similar: $(BIN)
	bash tests/test_similar_module.sh

.PHONY: test-similar


test-window-api: $(BIN)
	bash tests/test_window_api_module.sh

.PHONY: test-window-api


test-thumbnail: $(BIN)
	bash tests/test_thumbnail_module.sh

.PHONY: loogal-window test-window

loogal-window: src/gui/loogal_window.c include/gui/loogal_window.h loogal
	$(CC) $(CFLAGS) -Iinclude -o loogal-window src/gui/loogal_window.c $$(pkg-config --cflags --libs gtk4)

test-window:
	bash tests/test_window_build.sh
