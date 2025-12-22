ifndef C_ROOT
  $(error C_ROOT (Directory to put every C source files) is not defined. Please set it as an environment variable.)
endif

STACK_CC := gcc
STACK_CFLAGS := -Wall -Wextra -O2 -MMD -MP

STACK_SRC_DIR := $(C_ROOT)/stack/src
STACK_BIN_DIR := $(C_ROOT)/stack/bin
STACK_INCLUDE_DIR := $(C_ROOT)/stack/include
STACK_TEST_DIR := $(C_ROOT)/stack/tests

STACK_SRC_FILES := $(wildcard $(STACK_SRC_DIR)/*.c)
STACK_OBJ_FILES := $(patsubst $(STACK_SRC_DIR)/%.c,$(STACK_BIN_DIR)/%.o,$(STACK_SRC_FILES))
STACK_DEP_FILES := $(STACK_OBJ_FILES:.o=.d)

$(STACK_BIN_DIR)/libstack.a: $(STACK_OBJ_FILES)
	ar rcs $@ $^

.PHONY:
test_lstack: $(STACK_TEST_DIR)/test_lstack.c $(STACK_BIN_DIR)/libstack.a
	$(STACK_CC) $(STACK_CFLAGS) -o $(STACK_BIN_DIR)/test_lstack $^ $(shell pkg-config --cflags --libs stack concepts)

.PHONY:
test_vstack: $(STACK_TEST_DIR)/test_vstack.c $(STACK_BIN_DIR)/libstack.a
	$(STACK_CC) $(STACK_CFLAGS) -o $(STACK_BIN_DIR)/test_vstack $^ $(shell pkg-config --cflags --libs stack)

$(STACK_BIN_DIR)/%.o: $(STACK_SRC_DIR)/%.c | $(STACK_BIN_DIR)
	$(STACK_CC) $(STACK_CFLAGS) -I$(STACK_INCLUDE_DIR) -c $< -o $@

$(STACK_BIN_DIR):
	@mkdir -p $@

-include $(STACK_DEP_FILES)