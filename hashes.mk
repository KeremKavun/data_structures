ifndef PROJECT_ROOT
  $(error PROJECT_ROOT (Directory to put every C source files) is not defined. Please set it as an environment variable.)
endif

HASH_CC := gcc
HASH_CFLAGS := -Wall -Wextra -O2 -MMD -MP

HASH_SRC_DIR := $(PROJECT_ROOT)/hashes/src
HASH_BIN_DIR := $(PROJECT_ROOT)/hashes/bin
HASH_INCLUDE_DIR := $(PROJECT_ROOT)/hashes/include

HASH_SRC_FILES := $(wildcard $(HASH_SRC_DIR)/*.c)
HASH_OBJ_FILES := $(patsubst $(HASH_SRC_DIR)/%.c,$(HASH_BIN_DIR)/%.o,$(HASH_SRC_FILES))
HASH_DEP_FILES := $(HASH_OBJ_FILES:.o=.d)

$(HASH_BIN_DIR)/libhashes.a: $(HASH_OBJ_FILES)
	ar rcs $@ $^

$(HASH_BIN_DIR)/%.o: $(HASH_SRC_DIR)/%.c | $(HASH_BIN_DIR)
	$(HASH_CC) $(HASH_CFLAGS) -I$(HASH_INCLUDE_DIR) -c $< -o $@

$(HASH_BIN_DIR):
	@mkdir -p $@

-include $(HASH_DEP_FILES)