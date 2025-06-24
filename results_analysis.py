import pandas as pd
import sys
import os
# Get input file from command line argument if provided, otherwise use default
input_file = sys.argv[1] if len(sys.argv) > 1 else "benchmark_results.csv"

# Load benchmark results
df = pd.read_csv(input_file)

# Pivot to compare tabu and cplex
df_pivot = df.pivot_table(index=["size", "density", "repeat"], columns="solver", values="final_cost").reset_index()
df_pivot["gap_absolute"] = df_pivot["tabu"] - df_pivot["cplex"]
df_pivot["gap_relative"] = (df_pivot["gap_absolute"] / df_pivot["cplex"]).round(4)
# for visualization
df_pivot["gap_percent"] = (df_pivot["gap_relative"] * 100).round(2)

# Grouped summary for plotting
df_summary = df_pivot.groupby(["size", "density"]).agg({
    "gap_absolute": "mean",
    "gap_relative": "mean",
    "gap_percent": "mean"
}).reset_index()

# Save cleaned summary
summary_path = os.path.splitext(input_file)[0] + "_gap_summary.csv"
df_summary.to_csv(summary_path, index=False)
print("Saved summary to " + summary_path)