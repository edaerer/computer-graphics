CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude -I/usr/include
LDFLAGS = -lglfw -ldl -lGL -pthread -lassimp

SRC = src/glad.c main.cpp
OBJ = $(SRC:.cpp=.o)

all: fish_swim

fish_swim: $(SRC)
	$(CXX) $(CXXFLAGS) -o fish_swim $(SRC) $(LDFLAGS)

clean:
	rm -f fish_swim *.o
