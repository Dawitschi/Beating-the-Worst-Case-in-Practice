# AI-generated to get a quick first look at the data and get to know R.
# Run from the project root after ./release/cli/graph_properties:
# Rscript evaluation/bfs.R
# Reads graph properties + C++ CLI measurements. Writes PNGs only.
# Graph algorithms/statistics are not rerun here; means/ratios are plot analysis.
input_file <- "evaluation/bfs_results.csv"
properties_file <- "evaluation/plots/graph-summary.csv"
output <- "evaluation/plots"
# Explicit exploratory cutoff for the extreme wall-clock timings discussed.
# This is a plotting filter, not evidence that every excluded timing is invalid.
max_search_ms <- 10000
dir.create(output, recursive = TRUE, showWarnings = FALSE)
if (!file.exists(properties_file)) {
  stop("Missing graph-summary.csv: first run ./release/cli/graph_properties")
}

prepare_search_pairs <- function(results) {
  required <- c("graph", "source", "target", "algorithm", "search_ms", "found",
                "path_length", "expanded_nodes", "examined_edges", "graph_nodes", "graph_edges")
  if (!all(required %in% names(results))) stop("Missing required CSV columns")
  results <- subset(results, algorithm %in% c("bfs", "bibfs"))
  if (anyDuplicated(results[c("graph", "source", "target", "algorithm")])) {
    stop("Duplicate measurements for an algorithm and node pair")
  }
  paired <- merge(subset(results, algorithm == "bfs"),
                  subset(results, algorithm == "bibfs"),
                  by = c("graph", "source", "target"), suffixes = c("_bfs", "_bbfs"))
  if (nrow(paired) == 0) stop("No matching BFS/BBFS pairs")
  mismatch <- with(paired, found_bfs != found_bbfs |
                    (found_bfs == 1 & path_length_bfs != path_length_bbfs))
  if (any(mismatch)) warning("Algorithms disagree on reachability or path length")
  cat("Matched pairs:", nrow(paired), "; mismatches:", sum(mismatch), "\n")
  return(paired)
}

results <- read.csv(input_file)
paired <- prepare_search_pairs(results)
# Exclude the WHOLE node pair if either algorithm exceeds the cutoff, so both
# per-graph averages still use identical queries. The original CSV is untouched.
outlier_pairs <- with(paired,
  search_ms_bfs > max_search_ms | search_ms_bbfs > max_search_ms)
if (anyNA(outlier_pairs)) stop("Missing search timings in CLI results")
excluded_pairs <- sum(outlier_pairs)
extreme_measurements <- with(paired,
  sum(search_ms_bfs > max_search_ms) + sum(search_ms_bbfs > max_search_ms))
paired <- paired[!outlier_pairs, ]
if (nrow(paired) == 0) stop("No pairs remain after the timing filter")
cat("Excluded", extreme_measurements, "extreme timings across", excluded_pairs,
    "pairs; retaining", nrow(paired), "pairs\n")
filter_label <- sprintf("Timing filter: either algorithm > %g s; %d pairs excluded",
                        max_search_ms / 1000, excluded_pairs)
# Structural properties depend only on graph files; timings come only from CLI.
graph_properties <- read.csv(properties_file)
required_properties <- c("graph", "nodes", "edges", "average_degree", "max_degree",
                         "heterogeneity", "locality")
if (!all(required_properties %in% names(graph_properties)) ||
    anyDuplicated(graph_properties$graph)) {
  stop("Invalid graph-summary.csv: regenerate it with ./release/cli/graph_properties")
}
sizes <- unique(paired[c("graph", "graph_nodes_bfs", "graph_edges_bfs",
                         "graph_nodes_bbfs", "graph_edges_bbfs")])
if (anyDuplicated(sizes$graph)) stop("Inconsistent sizes in CLI results")
reference <- graph_properties[match(sizes$graph, graph_properties$graph), ]
if (anyNA(reference$graph) ||
    any(reference$nodes != sizes$graph_nodes_bfs) ||
    any(reference$edges != sizes$graph_edges_bfs) ||
    any(reference$nodes != sizes$graph_nodes_bbfs) ||
    any(reference$edges != sizes$graph_edges_bbfs)) {
  stop("Graph properties and CLI results do not describe the same graphs")
}
summary_columns <- c("search_ms_bfs", "search_ms_bbfs", "expanded_nodes_bfs",
                     "expanded_nodes_bbfs", "examined_edges_bfs", "examined_edges_bbfs")
graph_means <- aggregate(paired[summary_columns], list(graph = paired$graph), mean)
graph_means <- merge(graph_means, graph_properties, by = "graph")
# Retain the axis field names used by the existing plotting functions.
graph_means$graph_nodes_bfs <- graph_means$nodes
graph_means$graph_edges_bfs <- graph_means$edges
graph_means$time_speedup <- with(graph_means, search_ms_bfs / search_ms_bbfs)
graph_means$work_speedup <- with(graph_means, examined_edges_bfs / examined_edges_bbfs)
cat("Reading graph properties and CLI measurements for", nrow(graph_means), "graphs\n")

colors <- c(bfs = "#0072B2", bibfs = "#D55E00")
shapes <- c(bfs = 16, bibfs = 17)

save_plot <- function(filename, draw) {
  png(file.path(output, filename), width = 1400, height = 1100, res = 150)
  on.exit(dev.off())
  par(mar = c(5, 6, 4, 2) + 0.1, oma = c(1.5, 0, 0, 0),
      mgp = c(4, 1, 0), las = 1)
  draw()
  mtext(filter_label, side = 1, outer = TRUE, line = 0.2, cex = 0.65)
}

# Exploratory single-run measurements: execution order and machine activity matter.
# Lecture-style figures and the requested per-graph size comparison follow.

# Lecture-inspired figures (lecture02.pdf, slides 3 and 7).
# https://scale.iti.kit.edu/_media/teaching/2026ws/beating_wc/lecture02.pdf
# Here ONE DOT = ONE GRAPH, rather than one node pair.
# Means use exactly the same sampled pairs for both algorithms.
# "Search space" is shown as examined neighbor entries, our measured work count.
# We do not guess the slides' undefined mystery properties or asymptotic formula.
cat("Graphs in lecture-style plots:", nrow(graph_means), "\n")

save_plot("01-lecture-graph-parameters.png", function() {
  plot(graph_means$graph_nodes_bfs, graph_means$average_degree,
       log = "x", pch = 16, cex = 0.7, col = "#009E73",
       main = "Distribution of graph parameters",
       xlab = "Number of nodes (log scale)", ylab = "Average degree (2m / n)")
  grid()
})

# Smooth density across GRAPHS, not the degree distribution within one graph.
degree_density <- function(values, title, x_label, log_values = FALSE) {
  values <- values[is.finite(values)]
  if (log_values) values <- log10(values[values > 0])
  if (length(values) < 2 || length(unique(values)) < 2) {
    plot.new(); title(main = title); text(0.5, 0.5, "Not enough distinct observations")
    return(invisible(NULL))
  }
  d <- density(values)
  plot(d, type = "n", main = title, xlab = x_label,
       ylab = if (log_values) "Density per log10 unit" else "Density")
  grid()
  polygon(d, col = adjustcolor("#009E73", alpha.f = 0.55), border = "black")
  rug(values, col = adjustcolor("black", alpha.f = 0.12))
}
save_plot("02-lecture-average-degree-density.png", function() {
  degree_density(graph_means$average_degree, "Average degree across graphs", "Average degree")
})

# Continuous color encodes speedup, as in the lecture. Log2 units make large
# speedups readable: 0 = equal, 1 = twice as fast, 2 = four times as fast, etc.
color_scatter <- function(x, y, speedup, title, x_label, y_label, log_axes = "") {
  keep <- is.finite(x) & is.finite(y) & is.finite(speedup) & speedup > 0
  if (grepl("x", log_axes)) keep <- keep & x > 0
  if (grepl("y", log_axes)) keep <- keep & y > 0
  if (!any(keep)) stop("No valid observations for speedup plot")
  z <- log2(speedup[keep])
  z_range <- range(z)
  if (diff(z_range) == 0) z_range <- z_range + c(-0.5, 0.5)
  palette <- hcl.colors(256, "Plasma")
  color_index <- 1 + floor(255 * (z - z_range[1]) / diff(z_range))
  layout(matrix(c(1, 2), nrow = 1), widths = c(5, 1.2))
  par(mar = c(5, 6, 4, 1), mgp = c(3.8, 1, 0))
  plot(x[keep], y[keep], log = log_axes, pch = 16, cex = 0.75,
       col = palette[color_index], main = title, xlab = x_label, ylab = y_label)
  grid()
  # Restore dots above the grid.
  points(x[keep], y[keep], pch = 16, cex = 0.75, col = palette[color_index])
  par(mar = c(5, 1, 4, 5), mgp = c(3, 0.7, 0))
  levels <- seq(z_range[1], z_range[2], length.out = 256)
  image(c(0, 1), levels, matrix(rep(levels, each = 2), nrow = 2),
        col = palette, axes = FALSE, xlab = "", ylab = "")
  axis(4)
  mtext("log2 speedup", side = 3, line = 1, cex = 0.8)
  mtext("0 = equal; 1 = 2x; 2 = 4x", side = 4, line = 3, cex = 0.8, las = 0)
}

save_plot("03-lecture-time-speedup-linear.png", function() {
  color_scatter(graph_means$search_ms_bfs, graph_means$search_ms_bbfs,
                graph_means$time_speedup, "Speedup of bidirectional BFS",
                "Mean BFS time (ms)", "Mean BBFS time (ms)")
})
save_plot("04-lecture-time-speedup-log.png", function() {
  color_scatter(graph_means$search_ms_bfs, graph_means$search_ms_bbfs,
                graph_means$time_speedup, "Speedup of bidirectional BFS",
                "Mean BFS time (ms, log scale)", "Mean BBFS time (ms, log scale)", "xy")
})

save_plot("05-lecture-time-vs-search-work.png", function() {
  par(mfrow = c(1, 2), mar = c(6.5, 6.5, 4, 1), mgp = c(4.3, 1, 0), cex = 0.8)
  for (log_axes in c("", "xy")) {
    x <- c(graph_means$search_ms_bfs, graph_means$search_ms_bbfs)
    y <- c(graph_means$examined_edges_bfs, graph_means$examined_edges_bbfs)
    algorithms <- rep(c("bfs", "bibfs"), each = nrow(graph_means))
    keep <- is.finite(x) & is.finite(y)
    if (log_axes == "xy") keep <- keep & x > 0 & y > 0
    plot(x[keep], y[keep], log = log_axes, yaxt = "n",
         col = colors[algorithms[keep]], pch = shapes[algorithms[keep]], cex = 0.65,
         main = if (log_axes == "") "Mean search time and work" else "Same data: log scales",
         xlab = "Mean search time (ms)", ylab = "Mean examined neighbor entries")
    if (log_axes == "") {
      ticks <- pretty(range(y[keep]))
      axis(2, at = ticks, labels = format(ticks, scientific = TRUE, trim = TRUE))
    } else {
      axis(2)
    }
    grid()
    legend("topleft", legend = c("BFS", "BBFS"), col = colors,
           pch = shapes, bty = "n", cex = 0.85)
  }
})
save_plot("06-lecture-work-speedup.png", function() {
  color_scatter(graph_means$examined_edges_bfs, graph_means$examined_edges_bbfs,
                graph_means$work_speedup, "Reduction in search work",
                "Mean BFS neighbor entries (log scale)",
                "Mean BBFS neighbor entries (log scale)", "xy")
})

# Maximum-degree data were prepared by graph_properties.cpp.
if ("max_degree" %in% names(graph_means)) {
  save_plot("07-lecture-degree-distributions.png", function() {
    par(mfrow = c(1, 3), mar = c(5.5, 6.5, 4, 1), mgp = c(4.3, 1, 0), cex = 0.7)
    degree_density(graph_means$average_degree, "Average degree", "Average degree")
    degree_density(graph_means$max_degree, "Maximum degree", "Maximum degree")
    degree_density(graph_means$max_degree, "Maximum degree: log view",
                   "log10(maximum degree)", TRUE)
  })
} else {
  cat("Maximum-degree plots skipped: regenerate graph-summary.csv with ./release/cli/graph_properties\n")
}
cat("Saved lecture-inspired plots to:", output, "\n")

# Guessed mystery axes: estimated locality versus exact degree heterogeneity.
# Structure data are computed from the original graphs, not search timings.
# Structure is read from the graph-only CSV and joined to CLI timings in memory.
# Definitions: Section 2, doi:10.4230/LIPIcs.ESA.2022.21.
# H = log10(population degree standard deviation / average degree).
# L = (mean degree locality + max(0, mean distance locality)) / 2.
# Locality is estimated using up to 256 uniformly sampled non-bridge edges
# and 256 uniformly sampled nonedge pairs, both with replacement.
# Edge/nonedge distances are exact for those samples. Bridges are identified
# exactly and excluded. Fixed filename-derived seeds use base seed 42.
if (all(c("locality", "heterogeneity") %in% names(graph_means))) {
  properties <- graph_means
  # Descriptive finite-experiment cost exponent, not an asymptotic proof.
  properties$cost_exponent <- with(properties,
    log(examined_edges_bbfs) / log(edges))

  property_plane <- function(values, title, key_label, reverse = FALSE) {
    keep <- is.finite(properties$locality) &
      is.finite(properties$heterogeneity) & is.finite(values)
    z <- values[keep]
    if (!length(z)) stop("No finite values for mystery-property plot")
    z_range <- range(z)
    if (diff(z_range) == 0) z_range <- z_range + c(-0.5, 0.5)
    palette <- hcl.colors(256, "Plasma")
    if (reverse) palette <- rev(palette)
    index <- 1 + floor(255 * (z - z_range[1]) / diff(z_range))
    layout(matrix(c(1, 2), nrow = 1), widths = c(5, 1.3))
    par(mar = c(5, 6, 5, 1), mgp = c(3.8, 1, 0))
    plot(properties$locality[keep], properties$heterogeneity[keep],
         pch = 16, cex = 0.8, col = palette[index],
         xlab = "Estimated locality (guessed property A)",
         ylab = "Degree heterogeneity: log10(population SD / mean)", main = title)
    grid()
    points(properties$locality[keep], properties$heterogeneity[keep],
           pch = 16, cex = 0.8, col = palette[index])
    mtext(paste(sum(keep), "graphs; seeded locality sampling; exact heterogeneity"),
          side = 3, line = 0.5, cex = 0.75)
    par(mar = c(5, 1, 5, 4.5), mgp = c(3, 0.6, 0))
    levels <- seq(z_range[1], z_range[2], length.out = 256)
    image(c(0, 1), levels, matrix(rep(levels, each = 2), nrow = 2),
          col = palette, axes = FALSE, xlab = "", ylab = "")
    axis(4)
    mtext(key_label, side = 4, line = 2.8, cex = 0.8, las = 0)
  }

  save_plot("08-locality-heterogeneity-speedup.png", function() {
    property_plane(log2(properties$time_speedup),
                   "Graph structure and measured BBFS speedup",
                   "log2(BFS / BBFS time): 1 = 2x, 2 = 4x")
  })
  save_plot("09-locality-heterogeneity-cost-exponent.png", function() {
    property_plane(properties$cost_exponent,
                   "Graph structure and BBFS search cost",
                   "Cost exponent: log(mean examined entries) / log(m)", TRUE)
  })
  cat("Saved mystery-property plots for", nrow(properties), "graphs\n")
} else {
  cat("Mystery-property plots skipped: regenerate graph-summary.csv with ./release/cli/graph_properties\n")
}

# One dot per graph: ratio of mean runtimes over the SAME retained node pairs.
# Log axes show the full range without hiding large speedups. A ratio of 1
# means equal speed; above 1 means BBFS is faster. No trend is fitted because
# these graphs vary in structure as well as size.
save_plot("10-speedup-vs-graph-size.png", function() {
  keep <- with(graph_means, is.finite(nodes) & nodes > 0 &
                 is.finite(time_speedup) & time_speedup > 0)
  if (!any(keep)) stop("No valid observations for speedup versus graph size")
  size <- graph_means$nodes[keep]
  speedup <- graph_means$time_speedup[keep]
  plot(size, speedup, log = "xy", ylim = range(c(1, speedup)),
       pch = 16, cex = 0.7, col = adjustcolor(colors["bfs"], alpha.f = 0.6),
       main = "BBFS speedup versus graph size",
       xlab = "Number of nodes (log scale)",
       ylab = "Mean BFS time / mean BBFS time (log scale)")
  grid()
  points(size, speedup, pch = 16, cex = 0.7,
         col = adjustcolor(colors["bfs"], alpha.f = 0.6))
  abline(h = 1, col = "firebrick", lty = 2)
  legend("topleft", legend = "1 = equal speed; above = BBFS faster",
         col = "firebrick", lty = 2, bty = "n", cex = 0.8)
  mtext(paste(sum(keep), "graphs; one dot per graph"),
        side = 3, line = 0.4, cex = 0.8)
})
