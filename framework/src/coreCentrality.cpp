#include "framework/locality.hpp"

double CoreCentrality::computeLocality(const Graph &G) {
        return 1.0;
}

std::string CoreCentrality::localityName(){
        return "Example for Centrality measure";
}