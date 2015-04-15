all: sfmllint

sfmllint: main.o
	${CXX} -s -lboost_filesystem -lboost_system -o sfmllint main.o

main.o: main.cpp
	${CXX} -std=c++11 -pedantic -Wall -Wextra -fomit-frame-pointer -O3 -c main.cpp

clean:
	rm sfmllint main.o
