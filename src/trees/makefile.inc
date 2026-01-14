TREES_DIR     := src/trees
TREES_SOURCES := $(shell find $(TREES_DIR) -type f -name "*.c")
TREES_OBJS    := $(patsubst src/%, $(BIN_DIR)/%, $(TREES_SOURCES:.c=.o))

ALL_OBJS += $(TREES_OBJS)
ALL_TESTS += $(BIN_DIR)/tests/test_avl $(BIN_DIR)/tests/test_bst $(BIN_DIR)/tests/test_Btree $(BIN_DIR)/tests/test_heap

$(BIN_DIR)/tests/test_avl: tests/test_avl.c $(BIN_DIR)/$(LIB_NAME)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -L$(BIN_DIR) -lds -o $@

$(BIN_DIR)/tests/test_bst: tests/test_bst.c $(BIN_DIR)/$(LIB_NAME)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -L$(BIN_DIR) -lds -o $@

$(BIN_DIR)/tests/test_Btree: tests/test_Btree.c $(BIN_DIR)/$(LIB_NAME)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -L$(BIN_DIR) -lds -o $@

$(BIN_DIR)/tests/test_heap: tests/test_heap.c $(BIN_DIR)/$(LIB_NAME)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -L$(BIN_DIR) -lds -o $@

.PHONY: test_avl
test_avl: $(BIN_DIR)/tests/test_avl
	@echo "Running Avl Tree Test..."
	@./$<

.PHONY: test_bst
test_bst: $(BIN_DIR)/tests/test_bst
	@echo "Running Binary Search Tree Test..."
	@./$<

.PHONY: test_Btree
test_Btree: $(BIN_DIR)/tests/test_Btree
	@echo "Running B-tree Test..."
	@./$<

.PHONY: test_heap
test_heap: $(BIN_DIR)/tests/test_heap
	@echo "Running Heap Test..."
	@./$<