import pandas as pd
import sys
import os

# Get input file from command line argument if provided, otherwise use default
input_file = sys.argv[1] if len(sys.argv) > 1 else "benchmark_results.csv"

# Load benchmark results
df = pd.read_csv(input_file)

# Pivot to compare tabu and cplex final costs
df_pivot = df.pivot_table(index=["size", "density", "repeat"], columns="solver", values="final_cost").reset_index()
df_pivot["gap_absolute"] = df_pivot["tabu"] - df_pivot["cplex"]
df_pivot["gap_relative"] = df_pivot["gap_absolute"] / df_pivot["cplex"]
df_pivot["gap_percent"] = df_pivot["gap_relative"] * 100

# Grouped gap summary
df_summary = df_pivot.groupby(["size", "density"]).agg(
    gap_absolute_mean=("gap_absolute", "mean"),
    gap_relative_mean=("gap_relative", "mean"),
    gap_percent_mean=("gap_percent", "mean"),
    gap_percent_std=("gap_percent", "std"),
    count=("gap_percent", "count")
).reset_index()

# Calculate time gap directly without storing individual times
time_pivot = df.pivot_table(index=["size", "density", "repeat"], columns="solver", values="time_sec").reset_index()
time_pivot["time_gap_absolute"] = time_pivot["cplex"] - time_pivot["tabu"]

# Aggregate time gaps
time_summary = time_pivot.groupby(["size", "density"])["time_gap_absolute"].mean().reset_index()

# Merge time gaps into main summary
df_summary = pd.merge(df_summary, time_summary, on=["size", "density"], how="left")

# Save to CSV
summary_path = os.path.splitext(input_file)[0] + "_gap_summary.csv"
df_summary.to_csv(summary_path, index=False)
print(f"Saved summary to {summary_path}")