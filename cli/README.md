# Command-line programs

Run all examples from the project root. Build with:

```console
rtk cmake -S . -B release -DCMAKE_BUILD_TYPE=Release
rtk cmake --build release -j4
```

To rebuild only one program, use e.g.
`rtk cmake --build release --target bbfs -j4`.

## Original framework programs

| Program | Example | Behavior |
| --- | --- | --- |
| `run` | `rtk proxy ./release/cli/run instances/dolphins` | Loads a graph, extracts its largest connected component, and prints its node/edge counts and timings. |
| `convert_to_edgelist` | `rtk proxy ./release/cli/convert_to_edgelist instances/dolphins` | Loads a supported graph and writes an edge list beside it, replacing its extension with `.el`. |
| `graph_generator` | `rtk proxy ./release/cli/graph_generator -n 1000 -o generated.gr` | Generates a GIRG using the source's fixed model parameters and writes its largest connected component. Its final node count may be smaller than `-n`. |

These programs and `bbfs` accept `--help`. The conversion and generation
programs overwrite their output files.

## BFS and bidirectional BFS experiment

```console
rtk proxy ./release/cli/bbfs instances/graphs --nodes 10 --seed 42 --output evaluation/bfs_results.csv
```

| Argument | Meaning |
| --- | --- |
| `graph_directory` | Required directory; the CLI recursively collects `.gr` files and sorts their paths. |
| `-n`, `--nodes` | Required number of distinct sampled nodes per graph, at least 2. Uses all nodes if the graph is smaller. |
| `--seed` | Random seed, default `42`. Reproduces sampling with the same graph collection/order and runtime environment. |
| `-o`, `--output` | CSV output path, default `results.csv`. The R scripts expect `evaluation/bfs_results.csv`, so specify that path explicitly. |

For `k` sampled nodes, there are `k(k-1)/2` unordered pairs and twice that many
algorithm runs. Each pair is measured first with `bibfs`, then with `bfs`.

The framework `Timer` measures each call, including search initialization and
path reconstruction, in **milliseconds**. Graph loading has a separate timer;
CSV writing is outside the search timer. Timers accumulate, so each row records
the difference between the timer totals before and after that call.

CSV fields:

| Fields | Meaning |
| --- | --- |
| `graph`, `graph_nodes`, `graph_edges` | Relative graph filename and loaded graph size. |
| `sampled_nodes`, `seed`, `source`, `target`, `algorithm` | Sampling and query identity; algorithm labels are `bfs` and `bibfs`. |
| `load_ms`, `search_ms` | Graph loading and individual search duration. `load_ms` repeats for the graph's rows; do not sum it across queries. |
| `found` | `1` for a nonempty returned path, `0` otherwise. |
| `path_length`, `path` | Number of **nodes** and the space-separated node IDs. For a found path, edge distance is `path_length - 1`. |
| `expanded_nodes` | Nodes removed from the search queue(s), including an endpoint/meeting node popped before stopping. Not a count of distinct discovered vertices. |
| `examined_edges` | Neighbor entries examined. An undirected edge can contribute in both directions, and BBFS counts both searches. |

Each CLI run overwrites its CSV. The output directory must already exist.
An unsuccessful run can leave a partial experiment CSV. The ordinary BFS path
currently runs target-to-source; BBFS returns source-to-target.

The algorithm implementations and `SearchResult` are in
[`framework/src/bbfs.cpp`](../framework/src/bbfs.cpp) and
[`framework/include/framework/bbfs.hpp`](../framework/include/framework/bbfs.hpp).
BBFS alternates complete layers. The work-balanced variant from sheet 1 is
not implemented yet. Both searches currently use sets/maps for discovery state.

## Graph-property overview

```console
rtk proxy ./release/cli/graph_properties
```

No arguments are accepted. Fixed paths, relative to the project root:

- Input: `.gr` files directly inside `instances/graphs/` (not recursive).
- Output: `evaluation/plots/graph-summary.csv`.

The source is [`evaluation/graph_properties.cpp`](../evaluation/graph_properties.cpp),
registered as a CMake target in this directory. It uses the framework's `Graph`
and `ComponentStructure` classes and requires nonempty connected graphs.
Self-loops and duplicate edges are removed by `Graph`; ordinary cycles remain.

This program needs no BFS measurement CSV. It writes via a temporary file and
replaces the previous summary only after all graphs finish successfully.
See [evaluation/README.md](../evaluation/README.md) for property definitions
and the analysis workflow.

## Vertex cover

Run dominance reduction on every `.gr` file directly inside a directory, writing
`dominance_log.csv` in the current working directory:

```console
./release/cli/run_dominance instances/graphs/
```

For a single graph, results are printed to the console:

```console
 ./release/cli/run_dominance instances/graphs/1.gr
```

Compare Array, Hash Table, and Sorted Lists implementations on a directory,
writing `variants_log.csv` in the current working directory:

```console
./release/cli/run_dominance_with_diff_var instances/graphs/
```

Both batch commands overwrite their CSV. See
[evaluation/README.md](../evaluation/README.md#vertex-cover) for plotting the logs.
