import subprocess
import os
import csv
import time

# Paths to the executables for different solvers
solvers = {
    "tabu": "part2/main_tabu.out",
    "cplex": "part1/main_cplex.out"
}

# Path to board generator
generator_exe = "part1/generate_board.out"

# Output directory for .dat files
output_dir = "experiments"
os.makedirs(output_dir, exist_ok=True)

# Output CSV
csv_filename = "benchmark_results.csv"
first_write = True  # Track if header needs to be written

# Test configurations
sizes = [5, 10, 15, 20, 25, 30, 40, 50, 60]
densities = [0.05, 0.1, 0.15, 0.2, 0.3]
repeats = 3

results = []

for size in sizes:
    total_cells = size * size
    for density in densities:
        num_holes = int(total_cells * density)

        # Skip trivial or overly saturated boards
        if num_holes < 3 or num_holes > 0.6 * total_cells:
            continue

        for r in range(repeats):
            fname = f"{output_dir}/board_{size}_{num_holes}_{r}.dat"
            subprocess.run([generator_exe, str(size), str(num_holes), fname], check=True)

            for solver_name, solver_path in solvers.items():
                start = time.time()
                result = subprocess.run([solver_path, fname], capture_output=True, text=True)
                end = time.time()

                output = result.stdout
                lines = output.strip().split("\n")

                final_line = next((l for l in lines if l.startswith("FINAL_VALUE")), None)

                final_cost = float(final_line.split(":")[1].strip()) if final_line else -1

                results.append({
                    "solver": solver_name,
                    "size": size,
                    "density": density,
                    "holes": num_holes,
                    "repeat": r,
                    "filename": fname,
                    "final_cost": final_cost,
                    "time_sec": round(end - start, 4)
                })

    # Write intermediate results to file after each size
    write_mode = 'w' if first_write else 'a'
    with open(csv_filename, write_mode, newline='') as csvfile:
        writer = csv.DictWriter(csvfile, fieldnames=results[0].keys())
        if first_write:
            writer.writeheader()
            first_write = False
        writer.writerows(results)
    print(f"Results for size={size} saved to {csv_filename}")
    results.clear()  # Clear buffer after writing