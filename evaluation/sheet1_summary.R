# AI-generated to get a quick first look at the data and get to know R.
# Run from the project root: Rscript evaluation/sheet1_summary.R
# Exercise 2 asks for graphs with LITTLE OR NO ASYMPTOTIC SPEEDUP.
# https://scale.iti.kit.edu/_media/teaching/2026ws/beating_wc/sheet1.pdf
# This produces candidates from the current experiment, not proven classifications.
# IMPORTANT: our current BBFS alternates layers. The sheet requests choosing the
# direction by estimated layer work. Repeat the experiment with that variant
# before treating the candidate list as a final answer.

input_file <- "evaluation/bfs_results.csv"
properties_file <- "evaluation/plots/graph-summary.csv"
output_directory <- "evaluation/sheet1"

# Same paired timing filter as bfs.R. Raw input CSVs are never changed.
max_search_ms <- 10000
# Exploratory split suggested by the gap in the current speedup plot.
# A fixed threshold does NOT establish how speedup grows with graph size.
low_speedup_threshold <- 10

results <- read.csv(input_file)
required <- c("graph", "source", "target", "algorithm", "graph_nodes", "graph_edges",
              "search_ms", "found", "path_length", "expanded_nodes", "examined_edges")
if (!all(required %in% names(results))) stop("Missing columns in BFS results")
results <- subset(results, algorithm %in% c("bfs", "bibfs"))
if (anyDuplicated(results[c("graph", "source", "target", "algorithm")])) {
  stop("Duplicate algorithm/node-pair measurements")
}
paired <- merge(subset(results, algorithm == "bfs"),
                subset(results, algorithm == "bibfs"),
                by = c("graph", "source", "target"), suffixes = c("_bfs", "_bbfs"))
if (nrow(paired) == 0) stop("No matching BFS/BBFS pairs")
if (nrow(paired) * 2 != nrow(results)) stop("Some queries lack one algorithm's measurement")

sizes <- unique(paired[c("graph", "graph_nodes_bfs", "graph_edges_bfs",
                         "graph_nodes_bbfs", "graph_edges_bbfs")])
if (anyDuplicated(sizes$graph) ||
    any(sizes$graph_nodes_bfs != sizes$graph_nodes_bbfs) ||
    any(sizes$graph_edges_bfs != sizes$graph_edges_bbfs)) {
  stop("Inconsistent graph sizes")
}
if (any(!is.finite(paired$search_ms_bfs) | paired$search_ms_bfs <= 0 |
        !is.finite(paired$search_ms_bbfs) | paired$search_ms_bbfs <= 0)) {
  stop("Expected positive finite search timings")
}
paired$mismatch <- with(paired, found_bfs != found_bbfs |
                         (found_bfs == 1 & path_length_bfs != path_length_bbfs))
paired$excluded <- with(paired, search_ms_bfs > max_search_ms |
                         search_ms_bbfs > max_search_ms)
counts <- aggregate(cbind(total_pairs = rep(1, nrow(paired)),
                           excluded_pairs = as.integer(paired$excluded),
                           mismatched_pairs = as.integer(paired$mismatch)),
                     list(graph = paired$graph), sum)
retained <- paired[!paired$excluded, ]
if (length(unique(retained$graph)) != nrow(sizes)) {
  stop("At least one graph has no retained queries; inspect the timing cutoff")
}

# Ratios of per-graph means, using identical retained pairs for both algorithms.
columns <- c("search_ms_bfs", "search_ms_bbfs", "expanded_nodes_bfs",
             "expanded_nodes_bbfs", "examined_edges_bfs", "examined_edges_bbfs")
ranking <- aggregate(retained[columns], list(graph = retained$graph), mean)
names(ranking)[-1] <- paste0("mean_", columns)
medians <- aggregate(retained[c("search_ms_bfs", "search_ms_bbfs")],
                     list(graph = retained$graph), median)
names(medians)[-1] <- c("median_search_ms_bfs", "median_search_ms_bbfs")
ranking <- merge(merge(merge(ranking, medians, by = "graph"), counts, by = "graph"),
                 sizes[c("graph", "graph_nodes_bfs", "graph_edges_bfs")], by = "graph")
names(ranking)[names(ranking) == "graph_nodes_bfs"] <- "nodes"
names(ranking)[names(ranking) == "graph_edges_bfs"] <- "edges"
ranking$retained_pairs <- ranking$total_pairs - ranking$excluded_pairs
ranking$time_speedup <- with(ranking, mean_search_ms_bfs / mean_search_ms_bbfs)
ranking$work_speedup <- with(ranking, mean_examined_edges_bfs / mean_examined_edges_bbfs)
# The finite-instance exponent is descriptive, not an asymptotic proof.
ranking$bbfs_cost_exponent <- with(ranking, log(mean_examined_edges_bbfs) / log(edges))

properties <- read.csv(properties_file)
if (anyDuplicated(properties$graph)) stop("Duplicate graph-property records")
aligned <- properties[match(ranking$graph, properties$graph), ]
if (anyNA(aligned$graph) || any(aligned$nodes != ranking$nodes) ||
    any(aligned$edges != ranking$edges)) stop("Graph properties do not match measurements")
ranking$average_degree <- aligned$average_degree
ranking$max_degree <- aligned$max_degree
ranking$heterogeneity <- aligned$heterogeneity
ranking$estimated_locality <- aligned$locality

ranking$assessment <- ifelse(ranking$time_speedup <= low_speedup_threshold,
                             "low_measured_speedup_candidate", "larger_measured_speedup")
ranking$assessment[ranking$mismatched_pairs > 0] <- "inspect_correctness_mismatch"
ranking$low_speedup_threshold <- low_speedup_threshold
ranking$timing_cutoff_ms <- max_search_ms
ranking$implementation <- "alternating_layers_not_work_balanced"

# Slowest relative improvement first. Absolute runtime is recorded separately:
# a large graph can take longer while still benefiting greatly from BBFS.
ranking <- ranking[order(ranking$time_speedup), ]
ranking <- ranking[c("graph", "assessment", "time_speedup", "work_speedup",
                     "mean_search_ms_bfs", "mean_search_ms_bbfs",
                     "median_search_ms_bfs", "median_search_ms_bbfs",
                     "nodes", "edges", "retained_pairs", "excluded_pairs",
                     "total_pairs", "mismatched_pairs", "bbfs_cost_exponent",
                     "mean_expanded_nodes_bfs", "mean_expanded_nodes_bbfs",
                     "mean_examined_edges_bfs", "mean_examined_edges_bbfs",
                     "average_degree", "max_degree", "heterogeneity", "estimated_locality",
                     "low_speedup_threshold", "timing_cutoff_ms", "implementation")]
candidates <- ranking$graph[ranking$assessment == "low_measured_speedup_candidate"]
# Numeric ordering for 0.gr, 1.gr, ..., rather than 0.gr, 1.gr, 10.gr, ... .
ids <- suppressWarnings(as.numeric(tools::file_path_sans_ext(basename(candidates))))
candidates <- candidates[order(ids, candidates, na.last = TRUE)]

dir.create(output_directory, recursive = TRUE, showWarnings = FALSE)
write.csv(ranking, file.path(output_directory, "graph-ranking.csv"), row.names = FALSE)
# No header or comments: one filename per line, as requested by the sheet.
writeLines(candidates, file.path(output_directory, "exercise2-candidates.txt"))
cat("Saved ranking for", nrow(ranking), "graphs to", output_directory, "\n")
cat("Candidate graphs with <=", low_speedup_threshold, "x measured speedup:",
    length(candidates), "\n")
cat("Excluded", sum(counts$excluded_pairs), "pairs with either timing >",
    max_search_ms / 1000, "seconds\n")
cat("PROVISIONAL: current BBFS alternates layers; sheet requests work-balanced layers.\n")
cat("Exercise 3 (vertex cover) cannot be answered from BFS measurements.\n")
