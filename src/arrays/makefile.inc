ARRAYS_DIR     := src/arrays
ARRAYS_SOURCES := $(wildcard $(ARRAYS_DIR)/*.c)
ARRAYS_OBJS    := $(patsubst src/%, $(BIN_DIR)/%, $(ARRAYS_SOURCES:.c=.o))

ALL_OBJS  += $(ARRAYS_OBJS)
ALL_TESTS += $(BIN_DIR)/tests/test_dynarray

$(BIN_DIR)/tests/test_dynarray: tests/test_dynarray.c $(BIN_DIR)/$(LIB_NAME)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -L$(BIN_DIR) -lds -o $@

.PHONY: test_dynarray
test_dynarray: $(BIN_DIR)/tests/test_dynarray
	@echo "Running Dynamic Array Test..."
	@./$<