all:
	g++ -o exe src/*.cpp -lGL -lGLEW -lglfw

run: all
	./exe
