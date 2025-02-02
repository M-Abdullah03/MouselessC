# Makefile for Mouseless C++ project

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -Wextra -std=c++17

# Libraries
LIBS = -lgdi32 -luser32 -ldwmapi

# Source files
SRCS = ./src/main.cpp ./src/Overlay.cpp

# Header files
HDRS = ./src/Overlay.h

# Output executable
TARGET = ./bin/Mouseless

# Build target
all: $(TARGET)

# Link object files to create the executable
$(TARGET): $(SRCS) $(HDRS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

# Clean target
clean:
	rm -f $(TARGET)

# Phony targets
.PHONY: all clean