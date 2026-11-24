#include "framework/locality.hpp"

<<<<<<< HEAD
//Computes the density of a graph. We would assume that a graph is more local, if it has a higher density
double Density::computeLocality(const Graph &G, const unsigned sampleNodes) {
        return G.n() / G.m();
=======
double CoreCentrality::computeLocality(const Graph &G,  unsigned sampleNodes) {
        return 1.0;
>>>>>>> locality-setup
}

std::string Density::localityName(){
        return "Density";
}