ifndef C_ROOT
  $(error C_ROOT (Directory to put every C source files) is not defined. Please set it as an environment variable.)
endif

ARRAYS_CC := gcc
ARRAYS_CFLAGS := -Wall -Wextra -O2 -MMD -MP

ARRAYS_SRC_DIR := $(C_ROOT)/arrays/src
ARRAYS_BIN_DIR := $(C_ROOT)/arrays/bin
ARRAYS_INCLUDE_DIR := $(C_ROOT)/arrays/include
ARRAYS_TEST_DIR := $(C_ROOT)/arrays/tests

ARRAYS_SRC_FILES = $(shell find $(ARRAYS_SRC_DIR) -type f -name "*.c")
ARRAYS_OBJ_FILES = $(patsubst $(ARRAYS_SRC_DIR)/%.c, $(ARRAYS_BIN_DIR)/%.o, $(ARRAYS_SRC_FILES))
ARRAYS_DEP_FILES = $(ARRAYS_OBJ_FILES:.o=.d)

ARRAYS_SRC_DIRS = $(shell find $(ARRAYS_SRC_DIR) -type d)
ARRAYS_INTERNAL_DIRS = $(patsubst $(ARRAYS_SRC_DIR)/%, $(ARRAYS_BIN_DIR)/%, $(ARRAYS_SRC_DIRS))

# Static library
$(ARRAYS_BIN_DIR)/libarrays.a: $(ARRAYS_OBJ_FILES)
	ar rcs $@ $^

.PHONY:
test_dynarray: $(ARRAYS_TEST_DIR)/test_dynarray.c $(ARRAYS_BIN_DIR)/libarrays.a
	$(ARRAYS_CC) $(ARRAYS_CFLAGS) -o $(ARRAYS_BIN_DIR)/test_dynarray $^ $(shell pkg-config --cflags --libs concepts)

$(ARRAYS_OBJ_FILES): $(ARRAYS_BIN_DIR)/%.o: $(ARRAYS_SRC_DIR)/%.c | $(ARRAYS_INTERNAL_DIRS)
	$(ARRAYS_CC) $(ARRAYS_CFLAGS) -I$(ARRAYS_INCLUDE_DIR) -c $< -o $@

$(ARRAYS_INTERNAL_DIRS):
	@mkdir -p $@

-include $(ARRAYS_DEP_FILES)