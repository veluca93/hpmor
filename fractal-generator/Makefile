CC=clang++

all: build/flame

build/%: %.cc lodepng/lodepng.cpp lodepng/lodepng.h common.h
	${CC} -fopenmp -latomic -I lodepng $< -o $@ -Wall -O3 -g -ffast-math -march=native lodepng/lodepng.cpp

clean:
	rm -f build/*
