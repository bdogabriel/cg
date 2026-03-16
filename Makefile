all:
	g++ -o exe *.cpp -lGL -lGLEW -lglfw

run: all
	./exe
