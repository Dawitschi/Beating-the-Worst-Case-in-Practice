# Evaluation: BFS, graph overview, and vertex cover

All commands run from the **project root**, not from `evaluation/`. The analysis
scripts use fixed paths, base R, and no command-line arguments. RStudio is
optional; no extra R packages are needed for `bfs.R` or `sheet1_summary.R`.

The R analysis and graph-properties helper are marked as AI-generated for an
initial exploration and learning R.

## Workflow

Build the C++ programs:

```console
rtk cmake -S . -B release -DCMAKE_BUILD_TYPE=Release
rtk cmake --build release --target bbfs graph_properties -j4
```

Place the exercise graphs directly inside `instances/graphs/`. Measure the two
search algorithms on the same sampled pairs:

```console
rtk proxy ./release/cli/bbfs instances/graphs --nodes 10 --seed 42 --output evaluation/bfs_results.csv
```

`--nodes` is the number of distinct sampled nodes, not queries. Ten nodes mean
45 pairs and 90 search calls per graph. This command overwrites the measurement
CSV. See [cli/README.md](../cli/README.md) for arguments and measurement columns.

Generate the graph overview once when the graph collection changes:

```console
rtk proxy ./release/cli/graph_properties
```

This reads only the original graphs and writes `plots/graph-summary.csv`.
It does not read algorithm timings or invoke R. The fixed input directory is
`instances/graphs/`; only files directly inside it are considered.

Generate figures or the exercise-sheet list independently:

```console
rtk proxy Rscript evaluation/bfs.R
rtk proxy Rscript evaluation/sheet1_summary.R
```

Changing a plot does not require rerunning the searches or graph-property
calculation. R joins records by graph filename and checks node/edge counts.

## Inputs and outputs

Paths in this table are relative to the project root.

| File | Producer | Contents / consumer |
| --- | --- | --- |
| `evaluation/bfs_results.csv` | `bbfs` CLI | Per-query measurements and paths; both R scripts read it. |
| `evaluation/plots/graph-summary.csv` | `graph_properties` | One row of structural properties per graph; both R scripts read it. |
| `evaluation/plots/01-*.png` through `10-*.png` | `bfs.R` | Ten figures; plotting writes no CSVs. |
| `evaluation/sheet1/graph-ranking.csv` | `sheet1_summary.R` | All graphs sorted by increasing measured runtime speedup, with timings, work counts, graph properties, retained/excluded pair counts, and the classification rule. |
| `evaluation/sheet1/exercise2-candidates.txt` | `sheet1_summary.R` | Provisional low-speedup candidates, one filename per line, numerically ordered. |

The old intermediate files `graph-structure.csv`, `graph-structure-results.csv`,
`search-measurements.csv`, and `search-pairs.csv` are no longer needed or read.
There is no `graph_summary.R`; the C++ program writes the graph overview directly.

## Graph-property columns

`Graph` supplies the cleaned graph, degrees, neighbors, and edge lookup.
`ComponentStructure` checks connectivity. The summary contains no runtime or
speedup measurements.

| Columns | Meaning |
| --- | --- |
| `graph`, `nodes`, `edges` | Filename and graph size. |
| `average_degree`, `max_degree`, `degree_sd` | Exact degree statistics; average degree is `2m/n`, and SD is the population standard deviation. |
| `heterogeneity` | `log10(degree_sd / average_degree)`. Negative values are valid. |
| `degree_locality` | Estimated common-neighbor overlap, normalized by `min(degree(u), degree(v)) - 1`, over sampled non-bridge edges. |
| `distance_locality` | Estimated `max(0, 1 - (mean_detour - 2)/(mean_nonedge_distance - 2))`; defined as 0 when the reference mean is 2. |
| `locality` | Average of degree locality and distance locality. |
| `edge_samples`, `nonedge_samples`, `seed` | Actual sample counts and deterministic filename-derived seed (base seed 42). |

Locality follows [Section 2 of the locality/heterogeneity paper](https://drops.dagstuhl.de/opus/volltexte/2022/16959/pdf/LIPIcs-ESA-2022-21.pdf).
The helper samples up to 256 accepted non-bridge edges and 256 nonadjacent node
pairs, with replacement. A shortest-path distance helper ignores a sampled edge
to measure its detour; no detour means the edge is a bridge and is skipped.
Distances for the sampled queries are exact; the resulting graph-level locality
is an estimate. Edge sampling stops after at most 25,600 attempts.

Trees and complete graphs lack the samples required by this locality definition;
their locality is written as `NA`. Other undefined values are also `NA`, while
zero degree variation with a positive mean gives heterogeneity `-Inf`.
The graph-summary CSV is replaced only after a successful complete run.

## Figures

Each dot represents one graph. Search measurements are averaged over matching
node pairs retained for both algorithms. Runtime speedup is the **ratio of the
means**, not the mean of individual query speedups.

| Number | Figure |
| --- | --- |
| 01 | Number of nodes versus average degree. |
| 02 | Density of average degrees across graphs. |
| 03 | Mean BFS versus BBFS time, linear axes; color is log2 runtime speedup. |
| 04 | The same runtime comparison on logarithmic axes. |
| 05 | Mean search time versus mean examined neighbor entries, with linear and log panels; BFS is blue circles and BBFS orange triangles. |
| 06 | Mean BFS versus BBFS examined entries; color is log2 work reduction. |
| 07 | Average-/maximum-degree distributions, including a log10 maximum-degree view. |
| 08 | Estimated locality versus heterogeneity, colored by measured runtime speedup. The identification of the lecture's mystery axes remains an inference. |
| 09 | The same structural plane colored by `log(mean BBFS examined entries)/log(m)`, a descriptive cost exponent rather than an asymptotic proof. |
| 10 | Number of nodes versus runtime speedup; both axes logarithmic and a reference line at speedup 1. |

The style follows the [lecture figures](https://scale.iti.kit.edu/_media/teaching/2026ws/beating_wc/lecture02.pdf),
but measurements are from this implementation. Missing/non-finite values and
non-positive values on log axes cannot be plotted.

## Timing filter and interpretation

Both R scripts use the explicit cutoff `max_search_ms <- 10000`. If **either**
algorithm takes more than 10 seconds, the whole node pair is excluded before
computing either algorithm's averages. The raw CSV is not edited. Plot footers
show the cutoff and excluded-pair count. This exploratory cutoff removes the
observed extreme wall-clock timings; it does not establish why they occurred.

The current experiment measures each query once and always runs BBFS first.
Release builds still include the framework's sanitizer instrumentation. Timing
comparisons therefore remain exploratory. Examined-edge counts describe search
work independently of the measured wall-clock duration.

## Exercise sheet 1 export

The [sheet](https://scale.iti.kit.edu/_media/teaching/2026ws/beating_wc/sheet1.pdf)
asks, in exercise 2, for graphs with little or no **asymptotic** speedup.
`sheet1_summary.R` currently labels graphs with runtime speedup **at most 10x**
as candidates. Change `low_speedup_threshold` in that file to explore the split.
The CSV records the cutoff, algorithm variant, timing filter, correctness
mismatches, and retained/excluded query counts. A graph with a correctness
mismatch is flagged for inspection rather than put in the candidate list.

A fixed measured speedup threshold does not prove asymptotic behavior. Also,
the current BBFS alternates complete layers; the sheet requires selecting the
next direction by estimated layer work. The candidate list is provisional until
that implementation difference is addressed and the experiment is repeated.
Exercise 3 needs vertex-cover reduction results and cannot be answered from
these BFS measurements.

## Original R example

`example.R` and `babynames.csv` are the framework's separate demonstration.
Unlike the BFS scripts, this example requires `dplyr` and `ggplot2`. Install them
once from an R console with `install.packages(c("dplyr", "ggplot2"))`.
Run it with `evaluation/` as the working directory:

```console
rtk proxy Rscript -e 'setwd("evaluation"); source("example.R")'
```

It writes `evaluation/maleParkers.pdf` and `evaluation/interestingBabys.pdf`.

## Vertex cover

The dominance-reduction logs and plotting scripts are in `evaluation/vertex_cover/`.
See [the CLI documentation](../cli/README.md#vertex-cover) for generating
`dominance_log.csv` and `variants_log.csv`. Copy those logs into
`evaluation/vertex_cover/` before plotting.

Install the plotting dependencies:

```console
rtk pip install pandas matplotlib
```

Run the plots with `evaluation/vertex_cover/` as the working directory:

```console
rtk proxy python3 plot.py
rtk proxy python3 plot_var.py
```
