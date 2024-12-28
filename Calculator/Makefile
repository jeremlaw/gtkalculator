# Compiler and flags
CC = gcc
CFLAGS = -std=c11 `pkg-config --cflags gtk4`
LDFLAGS = `pkg-config --libs gtk4`

# Target executable
TARGET = calc

# Source files
SRCS = calc.c

# Build the executable
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

# Clean up build artifacts
clean:
	rm -f $(TARGET)