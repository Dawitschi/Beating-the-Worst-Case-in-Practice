#pragma once

#include <string>
#include "framework/graph.hpp"

class Locality {
public:
    virtual ~Locality() = default;
    virtual double computeLocality(const Graph& G, unsigned sampleNodes) = 0;
    virtual std::string localityName() = 0;
};

class Density : public Locality {
    public:
<<<<<<< HEAD
    ~Density() override = default;
=======
    ~CoreCentrality() override = default;
>>>>>>> locality-setup
    double computeLocality(const Graph &G,  unsigned sampleNodes) override ;
    std::string localityName() override;
};

//TODO: Write the header file for the respective centrality measure and add the corresponding cpp file also add it in cli/CMakeLists