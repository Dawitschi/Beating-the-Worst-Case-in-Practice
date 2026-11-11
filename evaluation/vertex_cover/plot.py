import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("dominance_log.csv")

df["ratio"] = df["largest_comp_size"] / df["orig_nodes"]

plt.figure(figsize=(10, 6))


sc = plt.scatter(
    df["orig_edges"], 
    df["largest_comp_size"], 
    c=df["ratio"], 
    cmap="plasma", 
    alpha=0.9, 
    s=40
)

cbar = plt.colorbar(sc)
cbar.set_label("Reduction Ratio (c/n)")

plt.xlabel("Number of edges")
plt.ylabel("Size of largest component")
plt.title("Vertex Cover Reduction")
plt.grid(True, linestyle="--", alpha=0.4)

plt.tight_layout()
plt.savefig("secret_subset_isolated.png", dpi=300)


#filter for extracting secret subset into txt file by deciding threshold(threshold is getting by looking at the plot)
secret_graphs = df[df["largest_comp_size"] <25]
with open("identified_graphs.txt", "w") as f:
    for filename in secret_graphs["filename"]:
        f.write(filename + "\n")


