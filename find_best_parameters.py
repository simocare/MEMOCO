import itertools
import shutil
import subprocess
import os
import csv
import time

# Paths
solver_path = "part2/main_tabu.out"
generator_exe = "part1/generate_board.out"

output_dir = "parameter_tuning"
os.makedirs(output_dir, exist_ok=True)

csv_filename = "tuning_results.csv"
first_write = True

# Config
sizes = [5, 10, 15, 20, 30]
densities = [0.05, 0.1, 0.15, 0.2]
repeats = 3

alphas = [0.5, 0.75, 1.0]
betas = [0.3, 0.5, 0.7]
decay_factors = [0.85, 0.9, 0.95]
lambdas = [0.0, 0.01, 0.05]

for size in sizes:
    total_cells = size * size
    for density in densities:
        num_holes = int(total_cells * density)

        if num_holes < 3 or num_holes > 0.6 * total_cells:
            continue

        for r in range(repeats):
            original_board_fname = f"{output_dir}/board_{size}_{num_holes}_{r}.dat"
            subprocess.run([generator_exe, str(size), str(num_holes), original_board_fname], check=True)

            best_result = None
            best_cost = float("inf")
            #log_files_to_clean = []

            for idx, (alpha, beta, decay_factor, lamb) in enumerate(itertools.product(alphas, betas, decay_factors, lambdas)):

                current_log_fname = f"{output_dir}/board_{size}_{num_holes}_{r}_run{idx}_log.txt"
                #log_files_to_clean.append(current_log_fname) # Add to cleanup list

                args = [
                    solver_path,
                    original_board_fname, # Pass the original board file
                    f"--alpha={alpha}",
                    f"--beta={beta}",
                    f"--decayFactor={decay_factor}",
                    f"--lambda={lamb}",
                    f"--logFile={current_log_fname}" # Tell the solver where to write its log
                ]

                start = time.time()
                result = subprocess.run(
                    args,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                    universal_newlines=True
                )
                if result.returncode != 0:
                    print(f"Solver failed for {original_board_fname} with parameters: alpha={alpha}, beta={beta}, decayFactor={decay_factor}, lambda={lamb}")
                    print("stderr:", result.stderr)
                    continue
                end = time.time()

                output = result.stdout
                lines = output.strip().split("\n")
                final_line = next((l for l in lines if l.startswith("FINAL_VALUE")), None)
                final_cost = float(final_line.split(":")[1].strip()) if final_line else -1

                if final_cost >= 0 and final_cost < best_cost:
                    best_cost = final_cost
                    best_result = {
                        "size": size,
                        "density": density,
                        "holes": num_holes,
                        "repeat": r,
                        "alpha": alpha,
                        "beta": beta,
                        "decayFactor": decay_factor,
                        "lambda": lamb,
                        "final_cost": final_cost,
                        "time_sec": round(end - start, 4),
                        "filename": original_board_fname,
                    }

            # Clean up all log files for this specific board/repeat after all parameter tunings
            # for log_file in log_files_to_clean:
            #     if os.path.exists(log_file):
            #         os.remove(log_file)
            # log_files_to_clean.clear()

            # Write only best_result for this repeat
            if best_result:
                write_mode = 'w' if first_write else 'a'
                with open(csv_filename, write_mode, newline='') as csvfile:
                    writer = csv.DictWriter(csvfile, fieldnames=best_result.keys())
                    if first_write:
                        writer.writeheader()
                        first_write = False
                    writer.writerow(best_result)

                print(f"Best tuning result for size={size}, density={density}, repeat={r} saved.")

            # Clean up generated files
            # if os.path.exists(original_board_fname):
            #     os.remove(original_board_fname)
