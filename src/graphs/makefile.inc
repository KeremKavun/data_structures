GRAPHS_DIR     := src/graphs
GRAPHS_SOURCES := $(wildcard $(GRAPHS_DIR)/*.c)
GRAPHS_OBJS    := $(patsubst src/%, $(BIN_DIR)/%, $(GRAPHS_SOURCES:.c=.o))

ALL_OBJS  += $(GRAPHS_OBJS)
ALL_TESTS += $(BIN_DIR)/tests/test_adjl_graph

$(BIN_DIR)/tests/test_adjl_graph: tests/test_adjl_graph.c $(BIN_DIR)/$(LIB_NAME)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -L$(BIN_DIR) -lds $(shell pkg-config --cflags --libs concepts) -o $@

.PHONY: test_adjl_graph
test_adjl_graph: $(BIN_DIR)/tests/test_adjl_graph
	@echo "Running Adjacency List Graph Test..."
	@./$<