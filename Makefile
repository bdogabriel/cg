all:
	g++ -Wall -Wextra -o exe src/*.cpp -lGL -lGLEW -lglfw

run: all
	./exe
