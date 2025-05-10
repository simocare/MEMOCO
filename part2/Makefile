CC = g++
CPPFLAGS = -g -Wall -O2
LDFLAGS =

OBJ = TSPSolver.o main.o

%.o: %.cpp
		$(CC) $(CPPFLAGS) -c $^ -o $@

main: $(OBJ)
		$(CC) $(CPPFLAGS) $(OBJ) -o main_tabu.out 
		
clean:
		rm -rf $(OBJ) main_tabu.out

.PHONY: clean
