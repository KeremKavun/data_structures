ifndef C_ROOT
  $(error C_ROOT (Directory to put every C source files) is not defined. Please set it as an environment variable.)
endif

GRAPHS_CC := gcc
GRAPHS_CFLAGS := -Wall -Wextra -O2 -MMD -MP

GRAPHS_SRC_DIR := $(C_ROOT)/graphs/src
GRAPHS_BIN_DIR := $(C_ROOT)/graphs/bin
GRAPHS_INCLUDE_DIR := $(C_ROOT)/graphs/include
GRAPHS_INTERNAL_DIR := $(C_ROOT)/graphs/internal
GRAPHS_TEST_DIR := $(C_ROOT)/graphs/tests

GRAPHS_SRC_FILES := $(shell find $(GRAPHS_SRC_DIR) -type f -name "*.c")
REL_SRC_FILES := $(patsubst $(GRAPHS_SRC_DIR)/%, %, $(GRAPHS_SRC_FILES))

# map to bin/
GRAPHS_OBJ_FILES := $(patsubst %.c, $(GRAPHS_BIN_DIR)/%.o, $(REL_SRC_FILES))
GRAPHS_DEP_FILES := $(GRAPHS_OBJ_FILES:.o=.d)

# mirror directory tree
GRAPHS_SRC_DIRS := $(shell find $(GRAPHS_SRC_DIR) -type d)
GRAPHS_BIN_DIRS := $(patsubst $(GRAPHS_SRC_DIR)/%, $(GRAPHS_BIN_DIR)/%, $(GRAPHS_SRC_DIRS))

$(GRAPHS_BIN_DIR)/libgraphs.a: $(GRAPHS_OBJ_FILES)
	ar rcs $@ $^

$(GRAPHS_BIN_DIR)/%.o: $(GRAPHS_SRC_DIR)/%.c | $(GRAPHS_BIN_DIRS)
	$(GRAPHS_CC) $(GRAPHS_CFLAGS) -I$(GRAPHS_INCLUDE_DIR) -I$(GRAPHS_INTERNAL_DIR) -c $< -o $@

$(GRAPHS_BIN_DIRS):
	@mkdir -p $@

-include $(GRAPHS_DEP_FILES)