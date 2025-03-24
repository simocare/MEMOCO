#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <set>

void generateBoard(int size, int num_holes, const std::string& filename) {
    std::vector<std::vector<int>> board(size, std::vector<int>(size, 0));
    std::set<std::pair<int, int>> used_positions;
    srand(time(nullptr));

    while (used_positions.size() < static_cast<size_t>(num_holes)) {
        int x = rand() % size;
        int y = rand() % size;
        if (used_positions.insert({x, y}).second) {
            board[y][x] = 1;
        }
    }

    std::ofstream outFile(filename);
    if (!outFile) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    outFile << size << "\n";
    for (const auto& row : board) {
        for (int cell : row) {
            outFile << cell << " ";
        }
        outFile << "\n";
    }

    outFile.close();
    std::cout << "Board saved to " << filename << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 3 || argc > 4) { // Allow 3 or 4 arguments
        std::cerr << "Usage: " << argv[0] << " <size> <num_holes> [output_filename]" << std::endl;
        return 1;
    }
    
    int size = std::stoi(argv[1]);
    int num_holes = std::stoi(argv[2]);

    std::string filename = (argc == 4) ? argv[3] : "board.dat"; // Use provided filename or default

    generateBoard(size, num_holes, filename);
    return 0;
}
