#pragma once

#include <string>
#include "framework/graph.hpp"

class Locality {
public:
    virtual ~Locality() = default;
    virtual double computeLocality(const Graph& G) = 0;
    virtual std::string localityName() = 0;
};

class CoreCentrality : public Locality {
    public:
    ~CoreCentrality() override = default;
    double computeLocality(const Graph &G) override ;
    std::string localityName() override;
};

//TODO: Write the header file for the respective centrality measure and add the corresponding cpp file also add it in cli/CMakeLists