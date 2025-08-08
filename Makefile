# Compiler and flags
CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# Source files
SOURCES = $(wildcard *.cpp)
# Object files (replace .cpp with .o)
OBJECTS = $(SOURCES:.cpp=.o)
# Executable names (replace .cpp with nothing)
EXECUTABLES = $(SOURCES:.cpp=)

# Default target - build all executables
all: $(EXECUTABLES)

# Railway deployment target - build only server
railway: server

# Rule to build each executable from its corresponding cpp file
%: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

# Clean up compiled files
clean:
	rm -f $(EXECUTABLES) $(OBJECTS)

# Phony targets
.PHONY: all clean railway

# Help target
help:
	@echo "Available targets:"
	@echo "  all     - Compile all C++ files"
	@echo "  railway - Compile server for Railway deployment"
	@echo "  clean   - Remove all compiled files"
	@echo "  help    - Show this help message"
	@echo ""
	@echo "Individual targets:"
	@echo "  client  - Compile client.cpp"
	@echo "  server  - Compile server.cpp"
	@echo "  test    - Compile test.cpp"
