import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

# === Load processed gap summary ===
df_summary = pd.read_csv("benchmark_gap_summary.csv")

# === Plot Gap Percentage ===
plt.figure(figsize=(12, 6))
sns.lineplot(data=df_summary, x="size", y="gap_percent", hue="density", marker="o")
plt.title("Tabu Gap Percentage Compared to CPLEX")
plt.xlabel("Board Size")
plt.ylabel("Gap (%)")
plt.grid(True)
plt.tight_layout()
plt.savefig("gap_percent_plot.png")
plt.close()

# === Load raw benchmark results for timing ===
df = pd.read_csv("benchmark_results.csv")
df_time = df.groupby(["solver", "size", "density"]).agg({"time_sec": "mean"}).reset_index()

# === Plot Timing ===
plt.figure(figsize=(12, 6))
sns.lineplot(data=df_time, x="size", y="time_sec", hue="solver", style="density", marker="o")
plt.title("Solver Runtime by Board Size and Density")
plt.xlabel("Board Size")
plt.ylabel("Time (seconds)")
plt.grid(True)
plt.tight_layout()
plt.savefig("runtime_plot.png")
plt.close()
