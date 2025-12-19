ifndef C_ROOT
  $(error C_ROOT (Directory to put every C source files) is not defined. Please set it as an environment variable.)
endif

LISTS_CC := gcc
LISTS_CFLAGS := -Wall -Wextra -O2 -MMD -MP

LISTS_SRC_DIR := $(C_ROOT)/lists/src
LISTS_BIN_DIR := $(C_ROOT)/lists/bin
LISTS_INCLUDE_DIR := $(C_ROOT)/lists/include
LISTS_TEST_DIR := $(C_ROOT)/lists/tests

LISTS_SRC_FILES = $(shell find $(LISTS_SRC_DIR) -type f -name "*.c")
LISTS_OBJ_FILES = $(patsubst $(LISTS_SRC_DIR)/%.c, $(LISTS_BIN_DIR)/%.o, $(LISTS_SRC_FILES))
LISTS_DEP_FILES = $(LISTS_OBJ_FILES:.o=.d)

LISTS_SRC_DIRS = $(shell find $(LISTS_SRC_DIR) -type d)
LISTS_INTERNAL_DIRS = $(patsubst $(LISTS_SRC_DIR)/%, $(LISTS_BIN_DIR)/%, $(LISTS_SRC_DIRS))

# Static library
$(LISTS_BIN_DIR)/liblists.a: $(LISTS_OBJ_FILES)
	ar rcs $@ $^

.PHONY:
test_clist: $(LISTS_TEST_DIR)/test_clist.c $(LISTS_BIN_DIR)/liblists.a
	$(LISTS_CC) $(LISTS_CFLAGS) -o $(LISTS_BIN_DIR)/test_clist $^ $(shell pkg-config --cflags --libs concepts)

.PHONY:
test_dlist: $(LISTS_TEST_DIR)/test_dlist.c $(LISTS_BIN_DIR)/liblists.a
	$(LISTS_CC) $(LISTS_CFLAGS) -o $(LISTS_BIN_DIR)/test_dlist $^ $(shell pkg-config --cflags --libs concepts)

.PHONY:
test_slist: $(LISTS_TEST_DIR)/test_slist.c $(LISTS_BIN_DIR)/liblists.a
	$(LISTS_CC) $(LISTS_CFLAGS) -o $(LISTS_BIN_DIR)/test_slist $^ $(shell pkg-config --cflags --libs concepts)

$(LISTS_OBJ_FILES): $(LISTS_BIN_DIR)/%.o: $(LISTS_SRC_DIR)/%.c | $(LISTS_INTERNAL_DIRS)
	$(LISTS_CC) $(LISTS_CFLAGS) -I$(LISTS_INCLUDE_DIR) -c $< -o $@

$(LISTS_INTERNAL_DIRS):
	@mkdir -p $@

-include $(LISTS_DEP_FILES)