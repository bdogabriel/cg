DIRS := math mesh render ui input command core io
SRC  := src/main.cpp $(foreach d,$(DIRS),$(wildcard src/$d/*.cpp))
INC  := $(foreach d,$(DIRS),-Isrc/$d)

BEAR ?= bear --

all:
	$(BEAR) g++ -Wall -Wextra -std=c++20 -o exe $(INC) $(SRC) -lGL -lGLEW -lglfw

run: all
	./exe
