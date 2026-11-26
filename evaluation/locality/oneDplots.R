library ( dplyr )
library ( tidyr )
library ( ggplot2 )

locality <- read.csv("locality.csv")
selectedLocality <- select(locality, graph, locality,localityMeasure)

bfs <- read.csv("bfs.csv")
vc <- read.csv("vc.csv")
bfs <- bfs %>% mutate(bfs = "bfs_good")
vc <- vc %>% mutate(vc, vc = "vc_good")

bfs <-c
vc <- merge(selectedLocality, vc, all=TRUE, by="graph")
bfs$bfs <- bfs$bfs |> replace_na("bfs_bad")
vc$vc <- vc$vc |> replace_na("vc_bad")
ggplot(bfs, aes(x = locality, y =localityMeasure, col= bfs)) +
  geom_point()

ggplot(vc, aes(x = locality, y =localityMeasure, col= vc)) +
  geom_point()



