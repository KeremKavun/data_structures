UTILS_DIR     := src/utils
UTILS_SOURCES := $(wildcard $(UTILS_DIR)/*.c)
UTILS_OBJS    := $(patsubst src/%, $(BIN_DIR)/%, $(UTILS_SOURCES:.c=.o))

ALL_OBJS  += $(UTILS_OBJS)