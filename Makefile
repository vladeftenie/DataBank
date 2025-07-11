CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude -pthread
LDFLAGS = -lzmq -lmysqlcppconn -pthread

COMMON_SRC = src/utils.cpp src/parse.cpp
COMMON_OBJ = $(COMMON_SRC:.cpp=.o)

TARGETS = server client admin

all: $(TARGETS)

server: src/server.cpp $(COMMON_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

client: src/client.cpp $(COMMON_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

admin: src/admin.cpp $(COMMON_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGETS) src/*.o logs/*.log
