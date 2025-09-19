ifndef PROJECT_ROOT
PROJECT_ROOT := C:/Users/KEREM/Desktop/code/C_codes
endif

HASH_TABLE_CC := gcc
HASH_TABLE_CFLAGS := -Wall -Wextra -O2 -MMD -MP

HASH_TABLE_SRC_DIR := $(PROJECT_ROOT)/hash_table/src
HASH_TABLE_BIN_DIR := $(PROJECT_ROOT)/hash_table/bin
HASH_TABLE_INCLUDE_DIR := $(PROJECT_ROOT)/hash_table/include

HASH_TABLE_SRC_FILES := $(wildcard $(HASH_TABLE_SRC_DIR)/*.c)
HASH_TABLE_OBJ_FILES := $(patsubst $(HASH_TABLE_SRC_DIR)/%.c,$(HASH_TABLE_BIN_DIR)/%.o,$(HASH_TABLE_SRC_FILES))
HASH_TABLE_DEP_FILES := $(HASH_TABLE_OBJ_FILES:.o=.d)

.SECONDEXPANSION:
$(HASH_TABLE_BIN_DIR)/libhashtable.a: $(HASH_TABLE_OBJ_FILES) $$(PRIME_BIN_DIR)/libprime.a
	ar rcs $@ $^

$(HASH_TABLE_BIN_DIR)/%.o: $(HASH_TABLE_SRC_DIR)/%.c | $(HASH_TABLE_BIN_DIR)
	$(HASH_TABLE_CC) $(HASH_TABLE_CFLAGS) -I$(HASH_TABLE_INCLUDE_DIR) -c $< -o $@

# Create bin directory
$(HASH_TABLE_BIN_DIR):
	@mkdir -p $@

-include $(HASH_TABLE_DEP_FILES)
include C:\Users\KEREM\Desktop\code\C_codes\prime\src\prime.mk