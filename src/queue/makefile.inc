QUEUE_DIR     := src/queue
QUEUE_SOURCES := $(wildcard $(QUEUE_DIR)/*.c)
QUEUE_OBJS    := $(patsubst src/%, $(BIN_DIR)/%, $(QUEUE_SOURCES:.c=.o))

ALL_OBJS  += $(QUEUE_OBJS)
ALL_TESTS += $(BIN_DIR)/tests/test_lqueue

$(BIN_DIR)/tests/test_lqueue: tests/test_lqueue.c $(BIN_DIR)/$(LIB_NAME)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC_FLAGS) $< -L$(BIN_DIR) -lds $(shell pkg-config --cflags --libs concepts) -o $@

.PHONY: test_lqueue
test_lqueue: $(BIN_DIR)/tests/test_lqueue
	@echo "Running List Queue Test..."
	@./$<