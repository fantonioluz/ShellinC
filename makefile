TARGET = shell
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pthread
BUILD_DIR = build

SRCS = shell.c comando.c strfco.c

OBJS = $(addprefix $(BUILD_DIR)/, $(SRCS:.c=.o))
DEPS = comando.h strfco.h

LDLIBS = -pthread

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD_DIR)/%.o: %.c $(DEPS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR)

-include $(OBJS:.o=.d)

$(BUILD_DIR)/%.d: %.c
	@mkdir -p $(BUILD_DIR)
	$(CC) -MM $(CFLAGS) $< | sed 's,\($*\)\.o[ :]*,$(BUILD_DIR)/\1.o $@ : ,g' > $@
