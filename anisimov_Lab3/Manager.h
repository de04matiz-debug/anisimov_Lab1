#pragma once
#include "Pipe.h"
#include "CompressorStation.h"
#include "GasNetwork.h"
#include <unordered_map>
#include <vector>

class Manager
{
    std::unordered_map<int, Pipe> pipes;
    std::unordered_map<int, CompressorStation> stations;
    int nextPipeId = 1;
    int nextStationId = 1;
    GasNetwork network;

    bool searchPipes();
    bool searchStations() const;
    bool managePipes(std::vector<int>& found);
    bool readNetwork(std::istream& in);

public:
    void showObjects() const;
    bool addPipe();
    bool addStation();
    bool editPipe();
    bool editStation();
    bool deleteObject();
    bool searchObjects();
    bool connectStations();
    bool disconnectPipe();
    void showNetwork() const;
    void sortNetwork() const;
    bool saveToFile(const std::string& fileName) const;
    bool loadFromFile(const std::string& fileName);
};
