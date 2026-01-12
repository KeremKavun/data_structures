ifndef C_ROOT
  $(error C_ROOT (Directory to put every C source files) is not defined. Please set it as an environment variable.)
endif

QUEUE_CC := gcc
QUEUE_CFLAGS := -Wall -Wextra -O2 -MMD -MP

QUEUE_SRC_DIR := $(C_ROOT)/queue/src
QUEUE_BIN_DIR := $(C_ROOT)/queue/bin
QUEUE_INCLUDE_DIR := $(C_ROOT)/queue/include
QUEUE_TEST_DIR := $(C_ROOT)/queue/tests

QUEUE_SRC_FILES := $(wildcard $(QUEUE_SRC_DIR)/*.c)
QUEUE_OBJ_FILES := $(patsubst $(QUEUE_SRC_DIR)/%.c,$(QUEUE_BIN_DIR)/%.o,$(QUEUE_SRC_FILES))
QUEUE_DEP_FILES := $(QUEUE_OBJ_FILES:.o=.d)

$(QUEUE_BIN_DIR)/libqueue.a: $(QUEUE_OBJ_FILES)
	ar rcs $@ $^

.PHONY:
test_lqueue: $(QUEUE_TEST_DIR)/test_lqueue.c $(QUEUE_BIN_DIR)/libqueue.a
	$(QUEUE_CC) $(QUEUE_CFLAGS) -o $(QUEUE_BIN_DIR)/test_lqueue $^ $(shell pkg-config --cflags --libs queue concepts)

$(QUEUE_BIN_DIR)/%.o: $(QUEUE_SRC_DIR)/%.c | $(QUEUE_BIN_DIR)
	$(QUEUE_CC) $(QUEUE_CFLAGS) -I$(QUEUE_INCLUDE_DIR) -c $< -o $@

$(QUEUE_BIN_DIR):
	@mkdir -p $@

-include $(QUEUE_DEP_FILES)