CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude -I/usr/include -I/usr/include/freetype2
LDFLAGS = -lglfw -ldl -lGL -pthread -lassimp -lfreetype

SRC = src/glad.c main.cpp
OBJ = $(SRC:.cpp=.o)

all: fish_swim

fish_swim: $(SRC)
	$(CXX) $(CXXFLAGS) -o fish_swim $(SRC) $(LDFLAGS)

clean:
	rm -f fish_swim *.o
