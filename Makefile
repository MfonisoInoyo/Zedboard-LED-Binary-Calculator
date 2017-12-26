# this makefile compiles our final project
# once made, its runnable file can be executed using the command 
# ./main 


main: main.o 
	g++ main.o -o main

main.o: final.cpp
	g++ -c final.cpp

clean:
	rm *.o main