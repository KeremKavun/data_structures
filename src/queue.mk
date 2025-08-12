ifndef PROJECT_ROOT
PROJECT_ROOT = C:/Users/KEREM/Desktop/code/C_codes
endif

QUEUE_CC := gcc
QUEUE_CFLAGS := -Wall -Wextra -MMD -MP

QUEUE_SRC_DIR := $(PROJECT_ROOT)/queue/src
QUEUE_BIN_DIR := $(PROJECT_ROOT)/queue/bin
QUEUE_INCLUDE_DIR := $(PROJECT_ROOT)/queue/include

QUEUE_SRC_FILES := $(wildcard $(QUEUE_SRC_DIR)/*.c)
QUEUE_OBJ_FILES := $(patsubst $(QUEUE_SRC_DIR)/%.c,$(QUEUE_BIN_DIR)/%.o,$(QUEUE_SRC_FILES))
QUEUE_DEP_FILES := $(QUEUE_OBJ_FILES:.o=.d)

# Static library
$(QUEUE_BIN_DIR)/libqueue.a: $(QUEUE_OBJ_FILES)
	ar rcs $@ $<

$(QUEUE_BIN_DIR)/%.o: $(QUEUE_SRC_DIR)/%.c | $(QUEUE_BIN_DIR)
	$(QUEUE_CC) $(QUEUE_CFLAGS) -I$(QUEUE_INCLUDE_DIR) -c $< -o $@

# Create bin directory
$(QUEUE_BIN_DIR):
	@mkdir -p $@

-include $(QUEUE_DEP_FILES)