import pandas as pd

# Load benchmark results
df = pd.read_csv("benchmark_results.csv")

# Pivot to compare tabu and cplex
df_pivot = df.pivot_table(index=["size", "density", "repeat"], columns="solver", values="final_cost").reset_index()
df_pivot["gap"] = df_pivot["tabu"] - df_pivot["cplex"]
df_pivot["accuracy"] = 1 - df_pivot["gap"] / df_pivot["cplex"]

# Grouped summary for plotting
df_summary = df_pivot.groupby(["size", "density"]).agg({
    "gap": "mean",
    "accuracy": "mean"
}).reset_index()

# Save cleaned summary
df_summary.to_csv("benchmark_accuracy_summary.csv", index=False)
print("Saved summary to benchmark_accuracy_summary.csv")