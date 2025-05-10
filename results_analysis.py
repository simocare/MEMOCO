import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os

# Load the latest benchmark results file
files = [f for f in os.listdir() if f.startswith("benchmark_results") and f.endswith(".csv")]
latest_file = max(files, key=os.path.getctime)
df = pd.read_csv(latest_file)

# Filter out rows with missing comparison data
df_pivot = df.pivot_table(index=["size", "density", "repeat"], columns="solver", values="final_cost").reset_index()
df_pivot["gap"] = df_pivot["tabu"] - df_pivot["cplex"]
df_pivot["accuracy"] = 1 - df_pivot["gap"] / df_pivot["cplex"]

# Group by size and density for plotting
df_summary = df_pivot.groupby(["size", "density"]).agg({"gap": "mean", "accuracy": "mean"}).reset_index()

# Plotting accuracy
plt.figure(figsize=(12, 6))
sns.lineplot(data=df_summary, x="size", y="accuracy", hue="density", marker="o")
plt.title("Accuracy of Tabu Search Compared to CPLEX")
plt.xlabel("Board Size")
plt.ylabel("Accuracy (1 - (Tabu - CPLEX) / CPLEX)")
plt.ylim(0.5, 1.05)
plt.grid(True)
plt.legend(title="Density")
plt.tight_layout()
plt.show()
