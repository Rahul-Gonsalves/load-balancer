CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic
TARGET = loadbalancer

SRCS = main.cpp Request.cpp WebServer.cpp LoadBalancer.cpp Switch.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
