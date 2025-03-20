import numpy as np
import matplotlib.pyplot as plt
import xml.etree.ElementTree as ET

# 🔹 Load the board from `board.dat`
def load_board(board_file):
    with open(board_file, 'r') as file:
        size = int(file.readline().strip())  # Read the single size value (square board)
        board = np.array([list(map(int, line.split())) for line in file])

    return board

# 🔹 Extract hole positions from the board
def extract_hole_positions(board):
    holes = []
    for y in range(board.shape[0]):
        for x in range(board.shape[1]):
            if board[y, x] == 1:
                holes.append((x, y))  # Store holes in (x, y) format
    return holes

# 🔹 Parse `drill.sol` to extract the optimal path
def load_solution(solution_file):
    tree = ET.parse(solution_file)
    root = tree.getroot()

    path = []
    for var in root.findall(".//variable"):
        name = var.get("name")
        value = float(var.get("value"))

        # Extract only `y_{i,j}` variables where value = 1
        if name.startswith("y_") and value == 1:
            _, from_node, to_node = name.split("_")
            path.append((int(from_node), int(to_node)))

    return path

# 🔹 Plot the board and solution path
def plot_board(board, path, title="Drilling Path"):
    holes = extract_hole_positions(board)
    
    fig, ax = plt.subplots(figsize=(8, 8))
    ax.set_xlim(-0.5, board.shape[1] - 0.5)
    ax.set_ylim(-0.5, board.shape[0] - 0.5)
    ax.set_xticks(range(board.shape[1]))
    ax.set_yticks(range(board.shape[0]))
    ax.set_xticklabels(range(board.shape[1]), fontsize=12)  # ⬆ Increase font size
    ax.set_yticklabels(range(board.shape[0]), fontsize=12)  # ⬆ Increase font size
    ax.grid(True, which='both', linestyle='--', color='gray', linewidth=0.5)
    ax.set_aspect("equal")  # ⬆ Make the grid square

    # 🔹 Plot holes with labels
    for i, (x, y) in enumerate(holes):
        ax.scatter(x, board.shape[0] - 1 - y, c='red', s=120, marker='o', edgecolors='black')
        ax.text(x, board.shape[0] - 1 - y, str(i), fontsize=14, ha='center', va='center', 
                color='white', bbox=dict(facecolor='black', alpha=0.6))

    # 🔹 Draw the drilling path with smaller arrows
    for (start, end) in path:
        x1, y1 = holes[start]
        x2, y2 = holes[end]
        ax.arrow(x1, board.shape[0] - 1 - y1, x2 - x1, -(y2 - y1), 
                 head_width=0.15, head_length=0.15, fc='blue', ec='blue', linewidth=2)

    ax.set_title(title, fontsize=14)
    plt.show()

# 🔹 Run the visualization
if __name__ == "__main__":
    board = load_board("board.dat")
    path = load_solution("drill.sol")
    plot_board(board, path)
