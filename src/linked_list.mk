ifndef PROJECT_ROOT
  $(error PROJECT_ROOT (Directory to put every C source files) is not defined. Please set it as an environment variable.)
endif

LINKED_LIST_CC := gcc
LINKED_LIST_CFLAGS := -Wall -Wextra -O2 -MMD -MP

LINKED_LIST_SRC_DIR := $(PROJECT_ROOT)/linked_lists/src
LINKED_LIST_BIN_DIR := $(PROJECT_ROOT)/linked_lists/bin
LINKED_LIST_INCLUDE_DIR := $(PROJECT_ROOT)/linked_lists/include

LINKED_LIST_SRC_FILES = $(shell find $(LINKED_LIST_SRC_DIR) -type f -name "*.c")
LINKED_LIST_OBJ_FILES = $(patsubst $(LINKED_LIST_SRC_DIR)/%.c, $(LINKED_LIST_BIN_DIR)/%.o, $(LINKED_LIST_SRC_FILES))
LINKED_LIST_DEP_FILES = $(LINKED_LIST_OBJ_FILES:.o=.d)

LINKED_LIST_SRC_DIRS = $(shell find $(LINKED_LIST_SRC_DIR) -type d)
LINKED_LIST_INTERNAL_DIRS = $(patsubst $(LINKED_LIST_SRC_DIR)/%, $(LINKED_LIST_BIN_DIR)/%, $(LINKED_LIST_SRC_DIRS))

# Static library
$(LINKED_LIST_BIN_DIR)/liblinkedlist.a: $(LINKED_LIST_OBJ_FILES)
	ar rcs $@ $^

$(LINKED_LIST_OBJ_FILES): $(LINKED_LIST_BIN_DIR)/%.o: $(LINKED_LIST_SRC_DIR)/%.c | $(LINKED_LIST_INTERNAL_DIRS)
	$(LINKED_LIST_CC) $(LINKED_LIST_CFLAGS) -I$(LINKED_LIST_INCLUDE_DIR) -c $< -o $@

$(LINKED_LIST_INTERNAL_DIRS):
	@mkdir -p $@

-include $(LINKED_LIST_DEP_FILES)