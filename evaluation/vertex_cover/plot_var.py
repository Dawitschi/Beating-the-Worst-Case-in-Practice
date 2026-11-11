import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("variants_log.csv")

plt.figure(figsize=(10, 6))

plt.scatter(df["orig_edges"], df["time_array"], label="Array", alpha=0.7, s=25, color="red")
plt.scatter(df["orig_edges"], df["time_hash"], label="Hash Table", alpha=0.7, s=25, color="blue")
plt.scatter(df["orig_edges"], df["time_sorted"], label="Sorted Lists", alpha=0.7, s=25, color="green")

plt.xlabel("Number of edges")
plt.ylabel("Processing Time (ms)")
plt.title("Performance Comparison on Various Dominance Reduction Variants")
plt.yscale("log")
plt.legend()
plt.grid(True, linestyle="--", alpha=0.4)

plt.tight_layout()
plt.savefig("variants_comparison.png", dpi=300)
