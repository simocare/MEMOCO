CC = g++
CPPFLAGS = -g -Wall -O2
LDFLAGS =

CPX_BASE    = /opt/ibm/ILOG/CPLEX_Studio2211
CPX_INCDIR  = $(CPX_BASE)/cplex/include
CPX_LIBDIR  = $(CPX_BASE)/cplex/lib/x86-64_linux/static_pic
CPX_LDFLAGS = -lcplex -lm -pthread -ldl

OBJ = main.o generate_board.o test_solver.o

%.o: %.cpp
		$(CC) $(CPPFLAGS) -I$(CPX_INCDIR) -c $^ -o $@

all: main generate_board test_solver

main: main.o
		$(CC) $(CPPFLAGS) main.o -o main_cplex.out -L$(CPX_LIBDIR) $(CPX_LDFLAGS)

generate_board: generate_board.o
		$(CC) $(CPPFLAGS) generate_board.o -o generate_board.out

test_solver: test_solver.o
		$(CC) $(CPPFLAGS) test_solver.o -o test_solver.out

clean:
		rm -rf $(OBJ) main_cplex.out generate_board.out test_solver.out

.PHONY: clean
