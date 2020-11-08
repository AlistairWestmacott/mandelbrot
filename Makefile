SRC_DIR = ./src
OBJ_DIR = ./obj
CCFLAGS = -Wall

SRC_FILES = $(wildcard src/*c)
OBJ_FILES = $(SRC_FILES:src/%.c=obj/%.o)

release: CCFLAGS += -O2 -march=native -DNDEBUG
release: mandelbrot
release: clean
debug: CCFLAGS += -fsanitize=address -fsanitize=undefined -g -fno-omit-frame-pointer -pedantic
debug: mandelbrot
debug: clean

LD_FLAGS = -lSDL2

obj/%.o: src/%.c
	clang $(CCFLAGS) $(SRC_DIR)/$(*F).c -o $(OBJ_DIR)/$(*F).o -c

mandelbrot: $(OBJ_FILES)
	clang $(LD_FLAGS) $(CCFLAGS) -o $@ $(OBJ_FILES)

clean: 
	rm obj/*
