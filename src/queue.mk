# Use PROJECT_ROOT if available, else define
ifndef PROJECT_ROOT
PROJECT_ROOT = C:/Users/KEREM/Desktop/code/C_codes
endif

QUEUE_SRC_DIR = $(PROJECT_ROOT)/queue/src
QUEUE_BIN_DIR = $(PROJECT_ROOT)/queue/bin
QUEUE_INCLUDE_DIR = $(PROJECT_ROOT)/queue/include

# Create bin directory if missing
$(QUEUE_BIN_DIR):
	@mkdir -p $@

# Static library
$(QUEUE_BIN_DIR)/libqueue.a: $(QUEUE_BIN_DIR)/queue.o
	ar rcs $@ $<

# Object file with directory dependency
$(QUEUE_BIN_DIR)/queue.o: $(QUEUE_SRC_DIR)/queue.c | $(QUEUE_BIN_DIR)
	$(CC) $(CFLAGS) -I$(QUEUE_INCLUDE_DIR) -c $< -o $@