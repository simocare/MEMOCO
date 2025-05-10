import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

# === Load processed accuracy summary ===
df_summary = pd.read_csv("benchmark_accuracy_summary.csv")

# === Plot Accuracy ===
plt.figure(figsize=(12, 6))
sns.lineplot(data=df_summary, x="size", y="accuracy", hue="density", marker="o")
plt.title("Accuracy of Tabu Search Compared to CPLEX")
plt.xlabel("Board Size")
plt.ylabel("Accuracy (1 - (Tabu - CPLEX) / CPLEX)")
plt.ylim(0.5, 1.05)
plt.grid(True)
plt.legend(title="Density")
plt.tight_layout()
plt.savefig("accuracy_plot.png")
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
