#pragma once

#include <string>
#include "framework/graph.hpp"

class Locality {
public:
    virtual ~Locality() = default;
    virtual double computeLocality(const Graph& G, unsigned sampleNodes, unsigned seed) = 0;
    virtual std::string localityName() = 0;
};

class Density : public Locality {
    public:
    ~Density() override = default;
    double computeLocality(const Graph &G, unsigned sampleNodes, unsigned seed) override;
    std::string localityName() override;
};

class ClosenessLocality : public Locality {
public:
    ~ClosenessLocality() override = default;
    double computeLocality(const Graph &G, unsigned sampleNodes, unsigned seed) override;
    std::string localityName() override;
};

//TODO: Write the header file for the respective centrality measure and add the corresponding cpp file also add it in cli/CMakeLists