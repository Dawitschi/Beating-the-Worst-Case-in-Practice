
#include <map>
#include <numeric>
#include <random>
#include <set>
#include "framework/bbfs.hpp"

#include "framework/locality.hpp"


std::string ClosenessLocality::localityName() {
    return "Closeness approximation";
}

/** This  locality measure approximates the average closeness of vertices. The Closeness centrality of a vertex v
 * indicates how close a vertex is to all other vertex in the graph.
 * Here, we sample same vertices and only consider the distance between these vertices.
 * To get a value for the whole graph, we divide by the number of sampled vertices.
*/
double ClosenessLocality::computeLocality(const Graph &G, unsigned sampleNodes, unsigned seed) {
    std::vector<node> sampled(G.n());
    std::iota(sampled.begin(), sampled.end(), node{0});
    std::mt19937 rng(seed);

    for (std::size_t i = 0; i < sampleNodes; ++i) {
        std::uniform_int_distribution<std::size_t> pick(
            i, sampled.size() - 1);
        std::swap(sampled[i], sampled[pick(rng)]);
    }
    //sample now contains all nodes of interest
    sampled.resize(sampleNodes);
    double summedClosenessCentrality = 0.0;
    unsigned distance;
    for (node root : sampled) {
        distance = 0;
        for (node target : sampled) {
            distance =+ bibfs(G,root,target).path_length;
        }
        summedClosenessCentrality =+ (G.n()-1)/distance;
    }
    return summedClosenessCentrality/sampleNodes;
}

