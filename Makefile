CC = g++
SRC = $(wildcard src/*.cpp) /usr/include/glad/glad.c
INCLUDES = -Iinclude
LIBS = -lglfw -ldl
OUT = out

all:
	$(CC) $(SRC) $(INCLUDES) $(LIBS) -o $(OUT)

run: all
	./$(OUT)

clean:
	rm -f $(OUT)