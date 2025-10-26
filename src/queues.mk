ifndef PROJECT_ROOT
  $(error PROJECT_ROOT (Directory to put every C source files) is not defined. Please set it as an environment variable.)
endif

QUEUE_CC := gcc
QUEUE_CFLAGS := -Wall -Wextra -O2 -MMD -MP

QUEUE_SRC_DIR := $(PROJECT_ROOT)/queue/src
QUEUE_BIN_DIR := $(PROJECT_ROOT)/queue/bin
QUEUE_INCLUDE_DIR := $(PROJECT_ROOT)/queue/include

QUEUE_SRC_FILES := $(wildcard $(QUEUE_SRC_DIR)/*.c)
QUEUE_OBJ_FILES := $(patsubst $(QUEUE_SRC_DIR)/%.c,$(QUEUE_BIN_DIR)/%.o,$(QUEUE_SRC_FILES))
QUEUE_DEP_FILES := $(QUEUE_OBJ_FILES:.o=.d)

.SECONDEXPANSION:
$(QUEUE_BIN_DIR)/libqueues.a: $(QUEUE_OBJ_FILES) $$(LINKED_LIST_BIN_DIR)/liblinkedlists.a
	ar x $(LINKED_LIST_BIN_DIR)/liblinkedlists.a
	ar rcs $@ $^ *.o
	rm -f *.o

$(QUEUE_BIN_DIR)/%.o: $(QUEUE_SRC_DIR)/%.c | $(QUEUE_BIN_DIR)
	$(QUEUE_CC) $(QUEUE_CFLAGS) -I$(QUEUE_INCLUDE_DIR) -c $< -o $@

# Create bin directory
$(QUEUE_BIN_DIR):
	@mkdir -p $@

-include $(QUEUE_DEP_FILES)
include $(PROJECT_ROOT)/linked_lists/src/linked_lists.mk