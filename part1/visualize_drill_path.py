import numpy as np
import matplotlib.pyplot as plt
import xml.etree.ElementTree as ET
import sys

# Load the board from the given file
def load_board(board_file):
    with open(board_file, 'r') as file:
        size = int(file.readline().strip())  # Read the single size value (square board)
        board = np.array([list(map(int, line.split())) for line in file])

    return board

# Extract hole positions from the board
def extract_hole_positions(board):
    holes = []
    for y in range(board.shape[0]):
        for x in range(board.shape[1]):
            if board[y, x] == 1:
                holes.append((x, y))  # Store holes in (x, y) format
    return holes

# Parse the solution file to extract the optimal path
def load_solution(solution_file):
    tree = ET.parse(solution_file)
    root = tree.getroot()

    # Extract the objective value from the <header> tag
    header = root.find("header")
    objective_value = float(header.get("objectiveValue")) if header is not None else None

    # Extract edges from y_i_j variables with value 1
    path = []
    for var in root.findall(".//variable"):
        name = var.get("name")
        value = float(var.get("value"))
        if name.startswith("y_") and value == 1:
            _, from_node, to_node = name.split("_")
            path.append((int(from_node), int(to_node)))

    return path, objective_value

# Plot the board and solution path
def plot_board(board, path, cost=None, title="Drilling Path"):
    holes = extract_hole_positions(board)

    fig, ax = plt.subplots(figsize=(8, 8))
    ax.set_xlim(-0.5, board.shape[1] - 0.5)
    ax.set_ylim(-0.5, board.shape[0] - 0.5)
    ax.set_xticks(range(board.shape[1]))
    ax.set_yticks(range(board.shape[0]))
    ax.set_xticklabels(range(board.shape[1]), fontsize=12)
    ax.set_yticklabels(range(board.shape[0]), fontsize=12)
    ax.grid(True, which='both', linestyle='--', color='gray', linewidth=0.5)
    ax.set_aspect("equal")

    for i, (x, y) in enumerate(holes):
        ax.scatter(x, board.shape[0] - 1 - y, c='red', s=120, marker='o', edgecolors='black')
        ax.text(x, board.shape[0] - 1 - y, str(i), fontsize=14, ha='center', va='center',
                color='white', bbox=dict(facecolor='black', alpha=0.6))

    for (start, end) in path:
        x1, y1 = holes[start]
        x2, y2 = holes[end]
        ax.arrow(x1, board.shape[0] - 1 - y1, x2 - x1, -(y2 - y1),
                 head_width=0.15, head_length=0.15, fc='blue', ec='blue', linewidth=2)

    # Add cost to the title
    if cost is not None:
        ax.set_title(f"{title}\nCost: {cost:.2f}", fontsize=14)
    else:
        ax.set_title(title, fontsize=14)

    plt.show()

# Main execution
if __name__ == "__main__":
    board_filename = "board.dat"
    solution_filename = "drill.sol"

    if len(sys.argv) > 1:
        base_name = sys.argv[1]
        board_filename = f"{base_name}.dat"
        solution_filename = f"{base_name}.sol"

    print(f"Loading board from: {board_filename}")
    print(f"Loading solution from: {solution_filename}")

    board = load_board(board_filename)
    path, cost = load_solution(solution_filename)
    plot_board(board, path, cost=cost, title=f"Best Solution for {board_filename}")
