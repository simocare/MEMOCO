import matplotlib.pyplot as plt
from matplotlib.widgets import Slider
import sys
import numpy as np

# --- Parse the log file ---
def parse_log_file(filepath):
    with open(filepath, 'r') as f:
        lines = f.readlines()

    tours = []
    values = []
    moves = []
    best_tour = []
    best_value = None
    starting_tour = []
    starting_value = None

    tour = []
    for line in lines:
        if line.startswith("TOUR") and not starting_tour:
            starting_tour = list(map(int, line.strip().split()[1:]))
        elif line.startswith("VALUE") and starting_value is None:
            starting_value = float(line.strip().split()[1])
        elif line.startswith("TOUR"):
            tour = list(map(int, line.strip().split()[1:]))
        elif line.startswith("VALUE"):
            values.append(float(line.strip().split()[1]))
            tours.append(tour.copy())
        elif line.startswith("MOVE"):
            parts = line.strip().replace(',', '').split()[1:]
            moves.append(tuple(map(int, parts)))
        elif line.startswith("FINAL_SOLUTION"):
            idx = lines.index(line)
            best_tour = list(map(int, lines[idx + 1].strip().split()))
        elif line.startswith("FINAL_VALUE"):
            best_value = float(line.strip().split()[1])

    if starting_tour and starting_value is not None:
        tours.insert(0, starting_tour)
        values.insert(0, starting_value)
    if best_tour and best_value is not None:
        tours.append(best_tour)
        values.append(best_value)

    return tours, values, moves

def load_board(board_file):
    with open(board_file, 'r') as file:
        size = int(file.readline().strip())
        board = np.array([list(map(int, line.split())) for line in file])
    return board

def extract_hole_positions(board):
    holes = []
    for y in range(board.shape[0]):
        for x in range(board.shape[1]):
            if board[y, x] == 1:
                holes.append((x, y)) # column, row
    return holes

# --- Plot function ---
def plot_tour(ax, coords, tour, value=None, move=None, title="", board_shape=(10, 10)):
    ax.clear()
    height = board_shape[0]

    # Setup axis layout and style
    ax.set_xlim(-0.5, board_shape[1] - 0.5)
    ax.set_ylim(-0.5, board_shape[0] - 0.5)
    ax.set_xticks(range(board_shape[1]))
    ax.set_yticks(range(board_shape[0]))
    ax.set_xticklabels(range(board_shape[1]), fontsize=12)
    ax.set_yticklabels(range(board_shape[0]), fontsize=12)
    ax.grid(True, which='both', linestyle='--', color='gray', linewidth=0.5)
    ax.set_aspect("equal")

    # Flip Y so origin is bottom-left
    flipped_coords = [(x, height - 1 - y) for (x, y) in coords]

    # Plot holes
    for i, (x, y) in enumerate(flipped_coords):
        ax.scatter(x, y, c='red', s=120, marker='o', edgecolors='black')
        ax.text(x, y, str(i), fontsize=14, ha='center', va='center',
                color='white', bbox=dict(facecolor='black', alpha=0.6))

    # Draw full tour
    for i in range(len(tour) - 1):
        start = flipped_coords[tour[i]]
        end = flipped_coords[tour[i + 1]]
        ax.plot([start[0], end[0]], [start[1], end[1]], 'blue', linewidth=2)
        ax.arrow(start[0], start[1], end[0] - start[0], end[1] - start[1],
                 head_width=0.15, head_length=0.15, fc='blue', ec='blue', length_includes_head=True)
    # Connect the last and first city
    start = flipped_coords[tour[-1]]
    end = flipped_coords[tour[0]]
    ax.plot([start[0], end[0]], [start[1], end[1]], 'blue', linewidth=2)
    ax.arrow(start[0], start[1], end[0] - start[0], end[1] - start[1],
             head_width=0.15, head_length=0.15, fc='blue', ec='blue', length_includes_head=True)

    # Highlight edges to be removed and update title
    if move:
        idx1_to_swap, idx2_to_swap = sorted(move)  # Ordina gli indici per coerenza
        node1_id = tour[idx1_to_swap]
        node2_id = tour[idx2_to_swap]
        title += f" (Swap Indices: {idx1_to_swap}-{idx2_to_swap})"

        try:
            # Identifica i nodi e gli indici coinvolti
            n_nodes = len(tour)
            node1_idx = idx1_to_swap
            node2_idx = idx2_to_swap

            # Trova gli indici dei nodi adiacenti ai segmenti che verranno riconnessi
            prev1_idx = (node1_idx - 1 + n_nodes) % n_nodes
            next1_idx = (node1_idx + 1) % n_nodes
            prev2_idx = (node2_idx - 1 + n_nodes) % n_nodes
            next2_idx = (node2_idx + 1) % n_nodes

            # Evidenzia i DUE archi "rimossi"
            start_removed1 = flipped_coords[tour[prev1_idx]]
            end_removed1 = flipped_coords[tour[node1_idx]]
            ax.plot([start_removed1[0], end_removed1[0]], [start_removed1[1], end_removed1[1]], 'red', linewidth=3)

            start_removed2 = flipped_coords[tour[node2_idx]]
            end_removed2 = flipped_coords[tour[next2_idx]]
            ax.plot([start_removed2[0], end_removed2[0]], [start_removed2[1], end_removed2[1]], 'red', linewidth=3)

        except ValueError:
            print(f"Warning: Indices {idx1_to_swap} or {idx2_to_swap} out of bounds for the current tour.")

    ax.set_title(f"{title}\nCost: {value:.2f}" if value is not None else title, fontsize=14)
    
# --- Main visualization with slider ---
def slider_visualization(log_file, dat_file):
    # Load board and extract hole positions
    board = load_board(dat_file)
    coords = extract_hole_positions(board)
    print("Coordinates of holes:", coords)

    board_shape = board.shape

    # Load log content
    tours, values, moves = parse_log_file(log_file)

    # Setup figure
    fig, ax = plt.subplots(figsize=(8, 8))
    plt.subplots_adjust(bottom=0.2)

    # Keyboard scroll
    current_idx = [0]
    def on_key(event):
        if event.key == 'right':
            current_idx[0] = min(current_idx[0] + 1, len(tours) - 1)
        elif event.key == 'left':
            current_idx[0] = max(current_idx[0] - 1, 0)
        elif event.key == 'home':
            current_idx[0] = 0
        elif event.key == 'end':
            current_idx[0] = len(tours) - 1
        else:
            return
        slider.set_val(current_idx[0])  # triggers the update automatically

    fig.canvas.mpl_connect('key_press_event', on_key)


    # Draw initial state
    plot_tour(ax, coords, tours[0], values[0], move=None,
              title="Initial Solution", board_shape=board_shape)

    # Setup slider
    ax_slider = plt.axes([0.2, 0.05, 0.65, 0.03])
    slider = Slider(ax_slider, 'Iteration', 0, len(tours) - 1, valinit=0, valstep=1)

    # Define slider update callback
    # Define slider update callback
    def update(val):
        idx = int(slider.val)

        title = ""
        move = None
        tour_idx = idx
        value_idx = idx

        if idx == 0:
            title = "Initial Solution"
        elif 0 < idx < len(tours) - 1:
            title = f"Iteration {idx}"
            if idx - 1 < len(moves):
                move = moves[idx - 1]
        elif idx == len(tours) - 1:
            title = "Best Solution"

        plot_tour(ax, coords, tours[tour_idx], values[value_idx], move, title=title, board_shape=board_shape)
        fig.canvas.draw_idle()

    slider.on_changed(update)
    plt.show()

# --- CLI interface ---
if __name__ == "__main__":
    if len(sys.argv) == 3:
        log_file = sys.argv[1]
        dat_file = sys.argv[2]
    elif len(sys.argv) == 2:
        dat_file = sys.argv[1]
        log_file = "tsp_log.txt"
    else:
        print("Usage: python visualize_ts.py tsp_log.txt board.dat")
        sys.exit(1)

    slider_visualization(log_file, dat_file)