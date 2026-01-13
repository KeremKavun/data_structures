STACK_DIR     := src/stack
STACK_SOURCES := $(wildcard $(STACK_DIR)/*.c)
STACK_OBJS    := $(patsubst src/%, $(BIN_DIR)/%, $(STACK_SOURCES:.c=.o))

ALL_OBJS  += $(STACK_OBJS)
ALL_TESTS += $(BIN_DIR)/tests/test_lstack

$(BIN_DIR)/tests/test_lstack: tests/test_lstack.c $(BIN_DIR)/$(LIB_NAME)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -L$(BIN_DIR) -lds $(shell pkg-config --cflags --libs concepts) -o $@

$(BIN_DIR)/tests/test_vstack: tests/test_vstack.c $(BIN_DIR)/$(LIB_NAME)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -L$(BIN_DIR) -lds $(shell pkg-config --cflags --libs concepts) -o $@

.PHONY: test_lstack
test_lstack: $(BIN_DIR)/tests/test_lstack
	@echo "Running List Stack Test..."
	@./$<

.PHONY: test_vstack
test_vstack: $(BIN_DIR)/tests/test_vstack
	@echo "Running Vector Stack Test..."
	@./$<