ifndef C_ROOT
  $(error C_ROOT (Directory to put every C source files) is not defined. Please set it as an environment variable.)
endif

TREES_CC := gcc
TREES_CFLAGS := -Wall -Wextra -O2 -MMD -MP

TREES_SRC_DIR := $(C_ROOT)/trees/src
TREES_BIN_DIR := $(C_ROOT)/trees/bin
TREES_INCLUDE_DIR := $(C_ROOT)/trees/include
TREES_INTERNAL_DIR := $(C_ROOT)/trees/internal
TREES_TEST_DIR := $(C_ROOT)/trees/tests

TREES_SRC_FILES := $(shell find $(TREES_SRC_DIR) -type f -name "*.c")
REL_SRC_FILES := $(patsubst $(TREES_SRC_DIR)/%, %, $(TREES_SRC_FILES))

# map to bin/
TREES_OBJ_FILES := $(patsubst %.c, $(TREES_BIN_DIR)/%.o, $(REL_SRC_FILES))
TREES_DEP_FILES := $(TREES_OBJ_FILES:.o=.d)

# mirror directory tree
TREES_SRC_DIRS := $(shell find $(TREES_SRC_DIR) -type d)
TREES_BIN_DIRS := $(patsubst $(TREES_SRC_DIR)/%, $(TREES_BIN_DIR)/%, $(TREES_SRC_DIRS))

$(TREES_BIN_DIR)/libtrees.a: $(TREES_OBJ_FILES)
	ar rcs $@ $^

.PHONY:
test_avl: $(TREES_TEST_DIR)/test_avl.c $(TREES_BIN_DIR)/libtrees.a
	$(TREES_CC) $(TREES_CFLAGS) -o $(TREES_BIN_DIR)/test_avl $^ $(shell pkg-config --cflags --libs trees concepts)

.PHONY:
test_bst: $(TREES_TEST_DIR)/test_bst.c $(TREES_BIN_DIR)/libtrees.a
	$(TREES_CC) $(TREES_CFLAGS) -o $(TREES_BIN_DIR)/test_bst $^ $(shell pkg-config --cflags --libs trees concepts)

.PHONY:
test_Btree: $(TREES_TEST_DIR)/test_Btree.c $(TREES_BIN_DIR)/libtrees.a
	$(TREES_CC) $(TREES_CFLAGS) -o $(TREES_BIN_DIR)/test_Btree $^ $(shell pkg-config --cflags --libs trees concepts)

.PHONY:
test_heap: $(TREES_TEST_DIR)/test_heap.c $(TREES_BIN_DIR)/libtrees.a
	$(TREES_CC) $(TREES_CFLAGS) -o $(TREES_BIN_DIR)/test_heap $^ $(shell pkg-config --cflags --libs trees concepts)

$(TREES_BIN_DIR)/%.o: $(TREES_SRC_DIR)/%.c | $(TREES_BIN_DIRS)
	$(TREES_CC) $(TREES_CFLAGS) -I$(TREES_INCLUDE_DIR) -I$(TREES_INTERNAL_DIR) -c $< -o $@

$(TREES_BIN_DIRS):
	@mkdir -p $@

-include $(TREES_DEP_FILES)