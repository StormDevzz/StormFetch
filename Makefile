CC = gcc
CFLAGS = -Wall -Wextra -O2 -Isrc
TARGET = stormfetch
SRCDIR = src
SOURCES = $(SRCDIR)/main.c $(SRCDIR)/info.c $(SRCDIR)/logo.c $(SRCDIR)/config.c $(SRCDIR)/util.c
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJECTS)

.PHONY: all clean
