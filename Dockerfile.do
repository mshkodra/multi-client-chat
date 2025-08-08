FROM gcc:11

WORKDIR /app

# Copy source files
COPY server.cpp .

# Build the server
RUN g++ -std=c++17 -Wall -Wextra -O2 -o server server.cpp

# Expose the port
EXPOSE 8080

# Start the server
CMD ["./server"] 