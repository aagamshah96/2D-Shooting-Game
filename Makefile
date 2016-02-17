all: sample2D

sample2D: Sample_GL3_2D.cpp glad.c
	g++ -o sample2D Sample_GL3_2D.cpp glad.c -lGL -lglfw -ldl -lftgl -lSOIL -I/usr/include/freetype2 -I/usr/local/lib/ -L/usr/local/lib

clean:
	rm sample2D
