CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude -Wextra -pthread
LIBS = -lzmq -pthread -lmysqlclient

SRC = $(wildcard src/*.cpp)
OBJ = $(SRC:.cpp=.o)
TARGETS = server client

all: $(BIN)

server: src/server.cpp
	$(CXX) $(CXXFLAGS) -o server src/server.cpp $(LIBS)

client: src/client.cpp
	$(CXX) $(CXXFLAGS) -o client src/client.cpp $(LIBS)

clean:
	rm -f $(TARGETS) src/*.o logs/*.log
