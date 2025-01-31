all: lab2

lab2: lab2.cpp
	g++ lab2.cpp libggfonts.a -Wall -o lab2 -lX11 -lGL -o lab2.out 

clean:
	rm -f lab2

