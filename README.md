# Beating the Worst Case -- Framework #

This framework serves as a starting point for the practical
course _"Beating the Worst Case in Practice: Unexpectedly Efficient Algorithms"_.  As a participant, you probably want to fork this
repository. As the repository contains submodules, you want to clone it with the `--recurse` flag.

``` console
git clone --recurse git@link_to_your_repository
```

The core of the framework is in [framework](framework/).
The [ext_libs](ext_libs/) folder contains the [CLI11](https://github.com/CLIUtils/CLI11) library for parsing command line options and the [GIRG](https://github.com/chistopher/girgs) library for generating graphs.
Unit tests are located in [test](test/) and use [googletest](https://github.com/google/googletest).
Moreover, [instances](./instances) includes some small instances for initial
testing.  The folder [evaluation](./evaluation) contains example data and an [R script](https://en.wikipedia.org/wiki/R_(programming_language)) to visualize it.
The BFS experiment, graph overview, plots, and exercise-sheet exports are documented below and in [evaluation/README.md](evaluation/README.md).


## Using the Framework ##

The repository contains a [.devcontainer](.devcontainer/devcontainer.json). If you have [Docker](https://www.docker.com/) installed, [VSCode](https://code.visualstudio.com/) can set up a fully working development environment on your platform inside a Docker container.
This can be especially useful for students working on windows.

If you don't want to use a Docker, you can build the project on your own using CMake.


## Compiling and Running ##

Run commands from the **project root**. Examples use the `rtk` wrapper from
this project's instructions; `rtk proxy` runs a command without filtering its output.

Configure and build all command-line programs in Release mode:

```console
rtk cmake -S . -B release -DCMAKE_BUILD_TYPE=Release
rtk cmake --build release -j4
```

For a Debug build, replace `release` with `debug` and `Release` with `Debug`.
The framework currently enables address/undefined-behavior sanitizers in both
configurations, so Release is optimized but still instrumented.

Try the original example on one graph:

```console
rtk proxy ./release/cli/run instances/dolphins
```

The available programs are `run`, `convert_to_edgelist`, `graph_generator`,
`bbfs`, `graph_properties`, `run_dominance`, and
`run_dominance_with_diff_var`. See [cli/README.md](cli/README.md) for their
arguments and outputs.

## BFS experiment and graph overview ##

Put the exercise collection's `.gr` files in `instances/graphs/`. For 10 sampled
nodes per graph, run:

```console
rtk proxy ./release/cli/bbfs instances/graphs --nodes 10 --seed 42 --output evaluation/bfs_results.csv
```

The CLI samples distinct nodes and runs BFS and BBFS on every unordered pair
among them. Thus 10 nodes give 45 pairs and 90 algorithm runs per graph; `--nodes`
is **not** the number of sampled pairs. Both algorithms receive the same pairs.
The output file is overwritten when the experiment starts.

Generate the separate graph-properties overview once when the graphs change:

```console
rtk proxy ./release/cli/graph_properties
```

This program has fixed paths and no command-line options. It reads only `.gr`
files directly inside `instances/graphs/` and writes
`evaluation/plots/graph-summary.csv`. It does not need BFS experiment results.
It uses the existing `Graph` class for loading and graph access. Degree
statistics are exact; locality is estimated from seeded samples.

With R installed, generate the ten figures and the exercise-2 candidate list:

```console
rtk proxy Rscript evaluation/bfs.R
rtk proxy Rscript evaluation/sheet1_summary.R
```

These scripts use base R, fixed project-relative paths, and no command-line
arguments. RStudio and additional R packages are not required. The analysis
code is marked as AI-generated for an initial exploration.

See [evaluation/README.md](evaluation/README.md) for CSV columns, plot names,
outlier filtering, candidate thresholds, and interpretation limits. The current
BBFS implementation alternates complete source/target layers; it does not yet
use the work-balanced direction choice requested by exercise sheet 1.

## Vertex cover ##

The vertex-cover exercise provides dominance reduction using arrays, hash tables,
and sorted lists. See [cli/README.md](cli/README.md#vertex-cover) for running
the programs and [evaluation/README.md](evaluation/README.md#vertex-cover)
for plotting the results.

## Graph Features ##

The framework contains a very basic graph data structure; see
[graph.hpp](framework/include/framework/graph.hpp) for more details.  For
examples on how to load a graph from a file or how to create a
subgraph, see [run.cpp](cli/run.cpp).  For an example of how to use
the graph data structure, see
[component_structure.cpp](framework/src/component_structure.cpp).

In addition to the graph data structure itself, there is the feature
of computing the connected components of a graph; see
[component_structure.hpp](framework/include/framework/component_structure.hpp)
for more details.  For an example of how to obtain the largest
connected component of a graph, see [run.cpp](cli/run.cpp).

## Supported Graph Formats ##

The following graph formats are supported. The loader should be able
to automatically figure out the format.

  * The graph format used by [konect.cc](http://konect.cc/networks/).  
    (TODO: verify if this still works)

  * The mtx format used by
    [networkrepository.com](https://networkrepository.com). Note that
    some graphs on
    [networkrepository.com](https://networkrepository.com) use a
    different format, which probably does not work.  There are plenty
    of graphs in the mtx format though.

  * A basic edge list with two integers per row.  Each integer
    represents a vertex and each row represents an edge between the
    two corresponding vertices.
	
The command line interface
[convert_to_edgelist.cpp](cli/convert_to_edgelist.cpp) reads a given
file and writes it as a basic edge list.

## Run Time Measurements ##

The `Timer` class provides the functionality of having globally
available named timers.  See [timer.hpp](framework/include/framework/timer.hpp)
for more details, and [run.cpp](cli/run.cpp) for an example of how to
use it.

## Parsing Command Line Arguments ##

[CLI11](https://github.com/CLIUtils/CLI11) is used for parsing
command line arguments.  It also automatically creates a help output;
for an example, run `rtk proxy ./release/cli/run --help` from the project root.  For a very basic example on how
to use it, see [run.cpp](cli/run.cpp).  Also see the [CLI11 GitHub
page](https://github.com/CLIUtils/CLI11#usage) for additional
information on how to use it.

## Running Unit Tests ##

[googletest](https://github.com/google/googletest) is already included in
the framework, providing easy unit testing.  After a Debug build, run the existing unit tests from the project root with:

```console
rtk proxy ctest --test-dir debug --output-on-failure
```

For an example of how to create test cases, see
[graph_tests.cpp](test/graph_tests.cpp).

## Adding Your Own Code ##

In addition to adding your header, source, main, and test files, you have to
tell CMake to use them.  For this, edit the
[CMakeLists.txt](CMakeLists.txt) files accordingly:

  * if you want to add your own command line application add a `.cpp`
    file to the folder `cli/` and to `set(EXECUTABLE_FILES ...)` in the [cli/CMakeLists.txt](cli/CMakeLists.txt)
  * `graph_properties` is registered separately in `cli/CMakeLists.txt` because its source is in `evaluation/graph_properties.cpp`.
  * if you want to modify the framework, add header and source files to
    `framework/include/framework` and `framework/src/` and add them to
    `set(HEADERS ...)` and `set(SOURCE_FILES ...)` in [framework/CMakeLists.txt](framework/CMakeLists.txt)
  * if you want to add unit test, add them to `test/` and set them in
    `set(TEST_FILES ...)` in [test/CMakeLists.txt](test/CMakeLists.txt).

