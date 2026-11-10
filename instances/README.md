# Graph instances

The files directly in this folder are small real-world networks originally
collected from [KONECT](https://konect.cc):

- `arenas-pgp`
- `brunson_south-africa_south-africa`
- `dolphins`
- `opsahl-powergrid`
- `petster-friendships-hamster-uniq`

Try one from the project root:

```console
rtk proxy ./release/cli/run instances/dolphins
```

## Exercise graph collection

Place the exercise collection's `.gr` files directly in `instances/graphs/`.
The supplied collection contains 672 graphs. The graph-property program reads
this fixed directory without recursion, so do not nest the files in another
folder. The BFS experiment CLI takes a directory argument and currently scans
it recursively; use the same flat collection for both programs.

```console
rtk proxy ./release/cli/bbfs instances/graphs --nodes 10 --seed 42 --output evaluation/bfs_results.csv
rtk proxy ./release/cli/graph_properties
```

The framework loads simple undirected, unweighted graphs: self-loops and duplicate
edges are removed, but cycles are retained. The graph overview requires each
input graph to be nonempty and connected.

See [cli/README.md](../cli/README.md) for CLI usage and
[evaluation/README.md](../evaluation/README.md) for the CSV overview and plots.
Additional networks can be obtained from [Network Repository](https://networkrepository.com);
see [download/](download/) for the original download helper.
