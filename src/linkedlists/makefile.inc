LIST_DIR     := src/linkedlists
LIST_SOURCES := $(shell find $(LIST_DIR) -type f -name "*.c")
LIST_OBJS    := $(patsubst src/%, $(BIN_DIR)/%, $(LIST_SOURCES:.c=.o))

ALL_OBJS  += $(LIST_OBJS)
ALL_TESTS += $(BIN_DIR)/tests/test_clist $(BIN_DIR)/tests/test_dlist $(BIN_DIR)/tests/test_slist

$(BIN_DIR)/tests/test_clist: tests/test_clist.c $(BIN_DIR)/$(LIB_NAME)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -L$(BIN_DIR) -lds $(shell pkg-config --cflags --libs concepts) -o $@

$(BIN_DIR)/tests/test_dlist: tests/test_dlist.c $(BIN_DIR)/$(LIB_NAME)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -L$(BIN_DIR) -lds $(shell pkg-config --cflags --libs concepts) -o $@

$(BIN_DIR)/tests/test_slist: tests/test_slist.c $(BIN_DIR)/$(LIB_NAME)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -L$(BIN_DIR) -lds $(shell pkg-config --cflags --libs concepts) -o $@

.PHONY: test_clist
test_clist: $(BIN_DIR)/tests/test_clist
	@echo "Running Intrusive and Circular Linked List Test..."
	@./$<

.PHONY: test_dlist
test_dlist: $(BIN_DIR)/tests/test_dlist
	@echo "Running Double Linked List Test..."
	@./$<

.PHONY: test_slist
test_slist: $(BIN_DIR)/tests/test_slist
	@echo "Running Singular Linked List Test..."
	@./$<