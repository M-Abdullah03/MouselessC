# Makefile for Mouseless C++ project

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -Wextra -std=c++17

# Libraries
LIBS = -lgdi32 -luser32 -ldwmapi

# Source files
SRCS = main.cpp Overlay.cpp

# Header files
HDRS = Overlay.h

# Output executable
TARGET = main

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