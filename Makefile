CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = stormfetch

all: $(TARGET)

$(TARGET): stormfetch.c
	$(CC) $(CFLAGS) -o $(TARGET) stormfetch.c

sysinfo: sysinfo.c
	$(CC) $(CFLAGS) -o sysinfo sysinfo.c

clean:
	rm -f $(TARGET) sysinfo

.PHONY: all clean
