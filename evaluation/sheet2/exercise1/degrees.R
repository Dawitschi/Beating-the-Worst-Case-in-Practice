
input_file <- "evaluation/sheet2/exercise1/degree-samples.csv"
output <- "evaluation/sheet2/exercise1/plots"
bfs <- "evaluation/bfs_results.csv"
vc <- "evaluation/vertex_cover/variants_log.csv"



dir.create(output, recursive = TRUE, showWarnings = FALSE)
if (!file.exists(input_file)) {
  stop("Missing degree-samples.csv")
}

bfs_results <- read.csv(bfs)
paired_bfs <- subset(
    merge(
        subset(bfs_results, algorithm == "bfs"),
        subset(bfs_results, algorithm == "bibfs"),
        by = c("graph", "source", "target"),
        suffixes = c("_bfs", "_bbfs")
    ),
    is.finite(search_ms_bfs) & is.finite(search_ms_bbfs) &
    search_ms_bfs <= 10000 & search_ms_bbfs <= 10000
)

graph_times_bfs <- aggregate(
  paired_bfs[c("search_ms_bfs", "search_ms_bbfs")],
  by = list(graph = paired_bfs$graph),
  FUN = mean
)
graph_times_bfs$speedup <- graph_times_bfs$search_ms_bfs / graph_times_bfs$search_ms_bbfs



results <- read.csv(input_file)
degrees <- lapply(trimws(results$degrees), function(x) {
  as.numeric(strsplit(x, "\\s+")[[1]])
})

results$mean_degree <- sapply(degrees, mean)
results$median_degree <- sapply(degrees, median)
results$degree_sd   <- sapply(degrees, sd)


idx <- match(results$graph, graph_times_bfs$graph)
results$speedup <- graph_times_bfs$speedup[idx]


save_plot <- function(filename, draw) {
  png(file.path(output, filename), width = 1400, height = 1100, res = 150)
  on.exit(dev.off())
  par(mar = c(5, 6, 4, 2) + 0.1, oma = c(1.5, 0, 0, 0),
      mgp = c(4, 1, 0), las = 1)
  draw()
}

cat("Lecture graphs")
N <- 10
colors <- rainbow(N)

degree_distributions <- lapply(degrees, function(d) {
  counts <- table(d)
  data.frame(
    degree = as.numeric(names(counts)),
    fraction = as.numeric(prop.table(counts))
  )
})

normalized_degrees <- lapply(seq_along(degree_distributions), function(i) {
  degree_distributions[[i]]$degree / results$mean_degree[i]
})



save_plot("01.1-lecture-graph-parameters.png", function() {
  plot(
    NA,
    log = "x",
    xlim = range(unlist(normalized_degrees)),
    ylim = c(0, max(unlist(lapply(degree_distributions, function(d) d$fraction)))),
    xlab = "Degree (normalized)",
    ylab = "Fraction of sampled vertices",
    main = "Degree distributions"
  )
  grid()
  for (i in seq_len(N)) {
    lines(
      x = degree_distributions[[i]]$degree/results$mean_degree[i],
      y = degree_distributions[[i]]$fraction,
      type = "b",
      col = colors[i],
      pch = 19
    )
  }

  legend(
    "topright",
    legend = results$graph[seq_len(N)],
    col = colors,
    lty = 1,
    pch = 19,
    cex = 0.7
  )
})

save_plot("01.2-lecture-graph-parameters.png", function() {
  plot(
    NA,
    log = "x",
    xlim = range(unlist(normalized_degrees)),
    ylim = c(0, max(unlist(lapply(degree_distributions, function(d) d$fraction)))),
    xlab = "Degree (normalized)",
    ylab = "Fraction of sampled vertices",
    main = "Degree distributions"
  )
  grid()
  for (i in seq_len(N)) {
    lines(
      x = degree_distributions[[i]]$degree/results$median_degree[i],
      y = degree_distributions[[i]]$fraction,
      type = "b",
      col = colors[i],
      pch = 19
    )
  }

  legend(
    "topright",
    legend = results$graph[seq_len(N)],
    col = colors,
    lty = 1,
    pch = 19,
    cex = 0.7
  )
})


save_plot("02-lecture-graph-parameters.png", function() {
  plot(
    NA,
    log = "x",
    xlim = range(unlist(normalized_degrees)),
    ylim = c(0, 1),
    xlab = "Degree (normalized)",
    ylab = "Fraction of sampled vertices",
    main = "Degree distributions"
  )
  grid()
  for (i in seq_len(N)) {
    lines(
      x = degree_distributions[[i]]$degree/results$mean_degree[i],
      y = 1 - cumsum(degree_distributions[[i]]$fraction),
      type = "b",
      col = colors[i],
      pch = 19
    )
  }

  legend(
    "topright",
    legend = results$graph[seq_len(N)],
    col = colors,
    lty = 1,
    pch = 19,
    cex = 0.7
  )
})

save_plot("03-lecture-graph-parameters.png", function() {
  plot(
    NA,
    log = "x",
    xlim = range(unlist(normalized_degrees)),
    ylim = c(0, 1),
    xlab = "Degree (normalized)",
    ylab = "Fraction of sampled vertices",
    main = "Degree distributions"
  )
  grid()
  for (i in seq_len(N)) {
    lines(
      x = degree_distributions[[i]]$degree/results$mean_degree[i],
      y = 1 - cumsum(degree_distributions[[i]]$fraction),
      type = "b",
      col = colors[i],
      pch = 19
    )
  }

  legend(
    "topright",
    legend = results$graph[seq_len(N)],
    col = colors,
    lty = 1,
    pch = 19,
    cex = 0.7
  )
})





palette <- hcl.colors(100, "Plasma")
z_range <- range(log2(results$speedup))
edges <- seq(z_range[1], z_range[2], length.out = 101)



save_plot("04-lecture-graph-parameters.png", function() {
    layout(matrix(c(1, 2), nrow = 1), widths = c(5, 1))
    par(mar = c(5, 6, 4, 1))
    plot(
        x = results$mean_degree,
        y = results$degree_sd,
        log = "yx",
        main = "Degree statistics per graph",
        xlab = "Mean sampled degree",
        ylab = "SD sampled degree",
        pch = 19,
        col = hcl.colors(100, "Plasma")[
            cut(log2(results$speedup), breaks = 100, labels = FALSE)
        ]
        )
    grid()
    par(mar = c(5, 1, 4, 4))
    image(
        x = c(0, 1),
        y = edges,
        z = matrix(seq_len(100), nrow = 1),
        col = palette,
        axes = FALSE,
        xlab = "",
        ylab = ""
    )
    ticks <- pretty(z_range)
    ticks <- ticks[ticks >= z_range[1] & ticks <= z_range[2]]
    axis(4, at = ticks, labels = paste0(signif(2^ticks, 3), "x"))
    box()
})

save_plot("05-lecture-graph-parameters.png", function() {
    layout(matrix(c(1, 2), nrow = 1), widths = c(5, 1))
    par(mar = c(5, 6, 4, 1))
    plot(
        x = results$mean_degree,
        y = results$degree_sd / results$mean_degree,
        log = "xy",
        main = "Relative degree variation",
        xlab = "Mean sampled degree",
        ylab = "SD / mean sampled degree",
        pch = 19,
        col = hcl.colors(100, "Plasma")[
            cut(log2(results$speedup), breaks = 100, labels = FALSE)
        ]  )
    grid()
    par(mar = c(5, 1, 4, 4))
    image(
        x = c(0, 1),
        y = edges,
        z = matrix(seq_len(100), nrow = 1),
        col = palette,
        axes = FALSE,
        xlab = "",
        ylab = ""
    )
    ticks <- pretty(z_range)
    ticks <- ticks[ticks >= z_range[1] & ticks <= z_range[2]]
    axis(4, at = ticks, labels = paste0(signif(2^ticks, 3), "x"))
    box()
})


save_plot("06-lecture-graph-parameters.png", function() {
    layout(matrix(c(1, 2), nrow = 1), widths = c(5, 1))
    par(mar = c(5, 6, 4, 1))
    plot(
        x = results$median_degree / results$mean_degree,
        y = results$degree_sd / results$mean_degree,
        log = "xy",
        main = "Relative degree variation",
        xlab = "Median sampled degree divided mean",
        ylab = "SD / mean sampled degree",
        pch = 19,
        col = hcl.colors(100, "Plasma")[
            cut(log2(results$speedup), breaks = 100, labels = FALSE)
        ]
        )
    grid()
    par(mar = c(5, 1, 4, 4))
    image(
        x = c(0, 1),
        y = edges,
        z = matrix(seq_len(100), nrow = 1),
        col = palette,
        axes = FALSE,
        xlab = "",
        ylab = ""
    )
    ticks <- pretty(z_range)
    ticks <- ticks[ticks >= z_range[1] & ticks <= z_range[2]]
    axis(4, at = ticks, labels = paste0(signif(2^ticks, 3), "x"))
    box()
})

