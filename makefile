CC        := gcc
CFLAGS    := -Iinclude -Wall -Wextra -O2 -g -MMD -MP

BIN_DIR := bin
LIB_NAME  := libds.a

ALL_OBJS  := 
ALL_TESTS := 

.PHONY: all clean test_all list_tests docs

all: $(BIN_DIR)/$(LIB_NAME)

include src/trees/makefile.inc
include src/stack/makefile.inc
include src/queue/makefile.inc
include src/linkedlists/makefile.inc
include src/hashs/makefile.inc
include src/arrays/makefile.inc
include src/graphs/makefile.inc
include src/utils/makefile.inc

$(BIN_DIR)/$(LIB_NAME): $(ALL_OBJS)
	@mkdir -p $(dir $@)
	@echo "Archiving $@"
	@ar rcs $@ $^

$(BIN_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

test_all: $(ALL_TESTS)
	@echo "Running all tests..."
	@for test in $(ALL_TESTS); do ./$$test; done

list_tests:
	@echo "Available specific tests:"
	@for test in $(ALL_TESTS); do \
		echo "  make $$(basename $$test)"; \
	done

.PHONY: docs
docs:
	@mkdir -p docs
	doxygen Doxyfile
	@echo "Documentation generated at docs/html/index.html"

clean:
	rm -rf $(BIN_DIR)

-include $(ALL_OBJS:.o=.d)