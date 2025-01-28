all:    program lab1 lab2

program: waitlist.cpp
	g++ waitlist.cpp -Wall -o program

lab1: lab1.cpp
	g++ lab1.cpp -Wall -lX11 -lGL -o lab1

lab2: lab2.cpp
	g++ lab2.cpp -Wall -o lab2


