library ( dplyr )
library ( tidyr )
library ( ggplot2 )

localities <- read.csv("locality.csv")
selectedLocality <- select(localities, graph, locality,localityMeasure)

bfs <- read.csv("bfs.csv")
vc <- read.csv("vc.csv")
bfs <- bfs %>% mutate(bfs = "bfs_good")
vc <- vc %>% mutate(vc, vc = "vc_good")

bfs <- merge(selectedLocality, bfs, all =TRUE, by="graph")
vc <- merge(selectedLocality, vc, all=TRUE, by="graph")
bfs$bfs <- bfs$bfs |> replace_na("bfs_bad")
vc$vc <- vc$vc |> replace_na("vc_bad")

bfs_results <-read.csv("../bfs_results.csv")
bfs_results = subset(bfs_results, search_ms < 10000)
paired <- merge(subset(bfs_results, algorithm == "bfs"),
                subset(bfs_results, algorithm == "bibfs"),
                by = c("graph", "source", "target"), suffixes = c("_bfs", "_bbfs"))
if (nrow(paired) == 0) stop("No matching BFS/BBFS pairs")

paired <-select(paired,graph,search_ms_bfs,search_ms_bbfs)

paired <-paired %>% group_by(graph) %>% 
  summarise(
    search_ms_bfs = mean(search_ms_bfs),
    search_ms_bbfs = mean(search_ms_bbfs)
  )

paired <- paired %>% mutate(spedup = search_ms_bbfs/search_ms_bfs)

bfs <- merge(bfs,paired,by="graph")
bfs_density = subset(bfs,localityMeasure == "Density")
ggplot(bfs_density, aes(x = locality, y =spedup)) +
  geom_point()+
  xlab("Density")
bfs_closeness = subset(bfs,localityMeasure == "Closeness approximation")
ggplot(bfs_closeness, aes(x = locality, y =spedup, col= bfs)) +
  geom_point()+
  xlab("Closeness approximation")


vc_result <- read.csv("../vertex_cover/dominance_log.csv")

vc_result <- vc_result %>% mutate(ratio = largest_comp_size/orig_nodes)
vc_result <- select(vc_result, ratio, filename)
vc_result <- vc_result %>% rename_at('filename', ~'graph')

vc <- merge(vc, vc_result, by="graph")
vc_density = subset(vc,localityMeasure == "Density")
ggplot(vc_density, aes(x = locality, y =ratio, col = vc)) +
  geom_point()+
  xlab("Density")
vc_closeness = subset(vc,localityMeasure == "Closeness approximation")
ggplot(vc_closeness, aes(x = locality, y =ratio, col= vc)) +
  geom_point()+
  xlab("Closeness approximation")



