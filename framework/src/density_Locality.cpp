#include "framework/locality.hpp"

//Computes the density of a graph. We would assume that a graph is more local, if it has a higher density
double Density::computeLocality(const Graph &G, const unsigned sampleNodes, unsigned seed) {
        return G.n() / G.m();
}

std::string Density::localityName(){
        return "Density";
}