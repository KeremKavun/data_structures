ifndef PROJECT_ROOT
  $(error PROJECT_ROOT (Directory to put every C source files) is not defined. Please set it as an environment variable.)
endif

TREES_CC := gcc
TREES_CFLAGS := -Wall -Wextra -O2 -MMD -MP

TREES_SRC_DIR := $(PROJECT_ROOT)/trees/src
TREES_BIN_DIR := $(PROJECT_ROOT)/trees/bin
TREES_INCLUDE_DIR := $(PROJECT_ROOT)/trees/include
TREES_INTERNAL_DIR := $(PROJECT_ROOT)/trees/internal

TREES_SRC_FILES := $(wildcard $(TREES_SRC_DIR)/*.c)
TREES_OBJ_FILES := $(patsubst $(TREES_SRC_DIR)/%.c,$(TREES_BIN_DIR)/%.o,$(TREES_SRC_FILES))
TREES_DEP_FILES := $(TREES_OBJ_FILES:.o=.d)

$(TREES_BIN_DIR)/libtrees.a: $(TREES_OBJ_FILES)
	ar rcs $@ $^

$(TREES_BIN_DIR)/%.o: $(TREES_SRC_DIR)/%.c | $(TREES_BIN_DIR)
	$(TREES_CC) $(TREES_CFLAGS) -I$(TREES_INCLUDE_DIR) -I$(TREES_INTERNAL_DIR) -c $< -o $@

$(TREES_BIN_DIR):
	@mkdir -p $@

-include $(TREES_DEP_FILES)
