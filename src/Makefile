# # CC = g++
# # CFLAGS = -g -pthread
# # TARGET = main
# # SRCS = main.cpp cache.cpp proxyhttp.cpp server.cpp HttpRequest.cpp client.cpp HttpResponse.cpp
# # OBJS = $(SRCS:.cpp=.o)

# # all: $(TARGET)

# # $(TARGET): main
# # 	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# all: main

# main: main.cpp httpproxy.hpp httpproxy.cpp  server.cpp server.hpp HttpRequest.cpp HttpRequest.hpp client.cpp client.hpp HttpResponse.hpp HttpResponse.cpp cache.cpp cache.hpp global.hpp logger.hpp
# 	g++ -g  -o main main.cpp httpproxy.cpp httpproxy.hpp server.cpp server.hpp HttpRequest.cpp HttpRequest.hpp client.cpp client.hpp HttpResponse.hpp HttpResponse.cpp cache.cpp cache.hpp global.hpp logger.hpp -lpthread

# .PHONY:
# 	clean
# clean:
# 	rm -rf *.o main

# Compiler
CC = g++
# Compiler flags
CFLAGS = -g -Wall -Wextra -std=c++11
# Libraries
LIBS = -lpthread

# Source files
SRCS = main.cpp proxyhttp.cpp server.cpp HttpRequest.cpp HttpResponse.cpp client.cpp cache.cpp
# Object files
OBJS = $(SRCS:.cpp=.o)
# Header files directory
INCDIR = .
# Executable name
TARGET = main

# Make all target
all: clean $(TARGET)

# Linking the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LIBS)

# Compile source files
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@ -I $(INCDIR)

# Clean target
.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)
