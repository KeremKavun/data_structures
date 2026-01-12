ifndef C_ROOT
  $(error C_ROOT (Directory to put every C source files) is not defined. Please set it as an environment variable.)
endif

LINKEDLISTS_CC := gcc
LINKEDLISTS_CFLAGS := -Wall -Wextra -O2 -MMD -MP

LINKEDLISTS_SRC_DIR := $(C_ROOT)/linkedlists/src
LINKEDLISTS_BIN_DIR := $(C_ROOT)/linkedlists/bin
LINKEDLISTS_INCLUDE_DIR := $(C_ROOT)/linkedlists/include
LINKEDLISTS_TEST_DIR := $(C_ROOT)/linkedlists/tests

LINKEDLISTS_SRC_FILES = $(shell find $(LINKEDLISTS_SRC_DIR) -type f -name "*.c")
LINKEDLISTS_OBJ_FILES = $(patsubst $(LINKEDLISTS_SRC_DIR)/%.c, $(LINKEDLISTS_BIN_DIR)/%.o, $(LINKEDLISTS_SRC_FILES))
LINKEDLISTS_DEP_FILES = $(LINKEDLISTS_OBJ_FILES:.o=.d)

LINKEDLISTS_SRC_DIRS = $(shell find $(LINKEDLISTS_SRC_DIR) -type d)
LINKEDLISTS_INTERNAL_DIRS = $(patsubst $(LINKEDLISTS_SRC_DIR)/%, $(LINKEDLISTS_BIN_DIR)/%, $(LINKEDLISTS_SRC_DIRS))

# Static library
$(LINKEDLISTS_BIN_DIR)/liblinkedlists.a: $(LINKEDLISTS_OBJ_FILES)
	ar rcs $@ $^

.PHONY:
test_clist: $(LINKEDLISTS_TEST_DIR)/test_clist.c $(LINKEDLISTS_BIN_DIR)/liblinkedlists.a
	$(LINKEDLISTS_CC) $(LINKEDLISTS_CFLAGS) -o $(LINKEDLISTS_BIN_DIR)/test_clist $^ $(shell pkg-config --cflags --libs concepts)

.PHONY:
test_dlist: $(LINKEDLISTS_TEST_DIR)/test_dlist.c $(LINKEDLISTS_BIN_DIR)/liblinkedlists.a
	$(LINKEDLISTS_CC) $(LINKEDLISTS_CFLAGS) -o $(LINKEDLISTS_BIN_DIR)/test_dlist $^ $(shell pkg-config --cflags --libs concepts)

.PHONY:
test_slist: $(LINKEDLISTS_TEST_DIR)/test_slist.c $(LINKEDLISTS_BIN_DIR)/liblinkedlists.a
	$(LINKEDLISTS_CC) $(LINKEDLISTS_CFLAGS) -o $(LINKEDLISTS_BIN_DIR)/test_slist $^ $(shell pkg-config --cflags --libs concepts)

$(LINKEDLISTS_OBJ_FILES): $(LINKEDLISTS_BIN_DIR)/%.o: $(LINKEDLISTS_SRC_DIR)/%.c | $(LINKEDLISTS_INTERNAL_DIRS)
	$(LINKEDLISTS_CC) $(LINKEDLISTS_CFLAGS) -I$(LINKEDLISTS_INCLUDE_DIR) -c $< -o $@

$(LINKEDLISTS_INTERNAL_DIRS):
	@mkdir -p $@

-include $(LINKEDLISTS_DEP_FILES)