#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <ctime>

void generateBoard(int size, int num_holes, const std::string& filename) {
    std::vector<std::vector<int>> board(size, std::vector<int>(size, 0));
    srand(time(nullptr));

    for (int i = 0; i < num_holes; ++i) {
        int x = rand() % size;
        int y = rand() % size;
        board[y][x] = 1; // Mark hole position
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
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <size> <num_holes>" << std::endl;
        return 1;
    }
    
    int size = std::stoi(argv[1]);
    int num_holes = std::stoi(argv[2]);
    
    generateBoard(size, num_holes, "board.dat");
    return 0;
}