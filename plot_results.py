import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import sys
import os

# Check for correct number of arguments
if len(sys.argv) not in [1, 3]:
    print("Error: Please provide either no arguments (for defaults) or exactly two file paths")
    print("Usage: python plot_results.py <gap_summary_file> <benchmark_file>")
    sys.exit(1)

# Set file paths based on arguments
if len(sys.argv) == 3:
    input_file = sys.argv[1]
    benchmark_file = sys.argv[2]
else:
    # Use defaults
    input_file = "benchmark_gap_summary.csv"
    benchmark_file = "benchmark_results.csv"

# === Load processed gap summary ===
df_summary = pd.read_csv(input_file)

# === Plot Gap Percentage ===
plt.figure(figsize=(12, 6))
sns.lineplot(data=df_summary, x="size", y="gap_percent_mean", hue="density", marker="o")
plt.title("Tabu Gap Percentage Compared to CPLEX")
plt.xlabel("Board Size")
plt.ylabel("Gap (%)")
plt.tight_layout()
gap_plot_path = os.path.splitext(input_file)[0] + "_gap_percent_plot.png"
plt.savefig(gap_plot_path)
plt.close()

# === Load raw benchmark results for timing ===
df = pd.read_csv(benchmark_file)
df_time = df.groupby(["solver", "size", "density"]).agg({"time_sec": "mean"}).reset_index()

# === Plot Timing ===
plt.figure(figsize=(12, 6))
sns.lineplot(data=df_time, x="size", y="time_sec", hue="solver", style="density", marker="o")
plt.title("Solver Runtime by Board Size and Density")
plt.xlabel("Board Size")
plt.tight_layout()
runtime_plot_path = os.path.splitext(input_file)[0] + "_runtime_plot.png"
plt.savefig(runtime_plot_path)
plt.close()