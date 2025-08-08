FROM gcc:11

# Set working directory
WORKDIR /app

# Copy source files
COPY server.cpp .

# Build the server
RUN g++ -std=c++17 -Wall -Wextra -O2 -o server server.cpp

# Create a non-root user for security
RUN useradd -m -u 1000 appuser && chown -R appuser:appuser /app
USER appuser

# Expose the port
EXPOSE 8080

# Start the server
CMD ["./server"] 