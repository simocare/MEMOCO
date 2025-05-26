import pandas as pd
import numpy as np

# Load data
df = pd.read_csv("tuning_results_complete_22_05.csv")

# Round density for grouping consistency
df["density"] = df["density"].round(4)

# Group by (size, density)
grouped = df.groupby(["size", "density"])

summary_rows = []

for (size, density), group in grouped:
    best_row = group.loc[group["final_cost"].idxmin()]
    best_cost = best_row["final_cost"]
    mean_cost = group["final_cost"].mean()
    std_cost = group["final_cost"].std()
    repeats = len(group)

    summary_rows.append({
        "size": size,
        "density": density,
        "repeats": repeats,
        "best_cost": best_cost,
        "mean_cost": round(mean_cost, 4),
        "std_cost": round(std_cost, 4),
        "alpha": best_row["alpha"],
        "beta": best_row["beta"],
        "decayFactor": best_row["decayFactor"],
        "lambda": best_row["lambda"],
    })

# Save summary
summary_df = pd.DataFrame(summary_rows)
summary_df.to_csv("summary_tuning.csv", index=False)
print("Saved summary to summary_tuning.csv")
