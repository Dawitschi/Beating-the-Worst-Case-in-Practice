# Vertex Cover: Dominance Rule & Subset Benchmarking


## Execution

### 1. Dominance Reduction with array implementation

Processes all `.gr` files and streams results live to `dominance_log.csv`:

```bash
./run_dominance /path/to/graphs/
```

Single-file test:
```bash
./run_dominance /path/to/graph.gr
```

### 2. Dominance Reduction with array implementation

Runs Array, Hash Table, and Sorted Lists methods sampled graphs and writes to
`variants_log.csv`:

```bash
./benchmark_dominance /path/to/graphs/ --max 100
```

---


