CXX = g++
CXXFLAGS = -Wall -O2 -g
INCLUDE_PATHS = -I. -Ipart1 -Ipart2

OBJS_FIND = find_best_parameters.o \
            part1/generate_board.o \
            part2/TSPSolver.o

OBJS_RUN = run_experiments.o \
           part1/generate_board.o \
           part2/TSPSolver.o

OUT_FIND = find_best_parameters.out
OUT_RUN = run_experiments.out

all: $(OUT_FIND) $(OUT_RUN)
$(OUT_FIND): $(OBJS_FIND)
	$(CXX) $(CXXFLAGS) $(INCLUDE_PATHS) -o $@ $^

$(OUT_RUN): $(OBJS_RUN)
	$(CXX) $(CXXFLAGS) $(INCLUDE_PATHS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE_PATHS) -c $< -o $@

clean:
	rm -f $(OBJS_FIND) $(OBJS_RUN) $(OUT_FIND) $(OUT_RUN)
