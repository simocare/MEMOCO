CXX = g++
CXXFLAGS = -Wall -O2 -g
INCLUDE_PATHS = -I. -Ipart1 -Ipart2

OBJS = find_best_parameters.o \
       part1/generate_board.o \
       part2/TSPSolver.o

OUT = find_best_parameters.out

all: $(OUT)

$(OUT): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDE_PATHS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE_PATHS) -c $< -o $@

clean:
	rm -f $(OBJS) $(OUT)