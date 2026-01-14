HASHS_DIR     := src/hashs
HASHS_SOURCES := $(wildcard $(HASHS_DIR)/*.c)
HASHS_OBJS    := $(patsubst src/%, $(BIN_DIR)/%, $(HASHS_SOURCES:.c=.o))

ALL_OBJS  += $(HASHS_OBJS)
ALL_TESTS += $(BIN_DIR)/tests/test_hash_table

$(BIN_DIR)/tests/test_hash_table: tests/test_hash_table.c $(BIN_DIR)/$(LIB_NAME)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -L$(BIN_DIR) -lds -o $@

.PHONY: test_hash_table
test_hash_table: $(BIN_DIR)/tests/test_hash_table
	@echo "Hash Table Test..."
	@./$<