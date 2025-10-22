ifndef PROJECT_ROOT
  $(error PROJECT_ROOT (Directory to put every C source files) is not defined. Please set it as an environment variable.)
endif

STACK_CC := gcc
STACK_CFLAGS := -Wall -Wextra -O2 -MMD -MP

STACK_SRC_DIR := $(PROJECT_ROOT)/stack/src
STACK_BIN_DIR := $(PROJECT_ROOT)/stack/bin
STACK_INCLUDE_DIR := $(PROJECT_ROOT)/stack/include

STACK_SRC_FILES := $(wildcard $(STACK_SRC_DIR)/*.c)
STACK_OBJ_FILES := $(patsubst $(STACK_SRC_DIR)/%.c,$(STACK_BIN_DIR)/%.o,$(STACK_SRC_FILES))
STACK_DEP_FILES := $(STACK_OBJ_FILES:.o=.d)

# Static library
$(STACK_BIN_DIR)/libstacks.a: $(STACK_OBJ_FILES)
	ar rcs $@ $^

$(STACK_BIN_DIR)/%.o: $(STACK_SRC_DIR)/%.c | $(STACK_BIN_DIR)
	$(STACK_CC) $(STACK_CFLAGS) -I$(STACK_INCLUDE_DIR) -c $< -o $@

# Create bin directory
$(STACK_BIN_DIR):
	@mkdir -p $@

-include $(STACK_DEP_FILES)