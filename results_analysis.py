import pandas as pd

# Load benchmark results
df = pd.read_csv("benchmark_results.csv")

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
df_summary.to_csv("benchmark_gap_summary.csv", index=False)
print("Saved summary to benchmark_gap_summary.csv")