#pragma once
#include "Pipe.h"
#include "CompressorStation.h"
#include <unordered_map>
#include <vector>

class Manager
{
    std::unordered_map<int, Pipe> pipes;
    std::unordered_map<int, CompressorStation> stations;
    int nextPipeId = 1;
    int nextStationId = 1;

    bool searchPipes();
    bool searchStations() const;
    bool managePipes(std::vector<int>& found);

public:
    void showObjects() const;
    bool addPipe();
    bool addStation();
    bool editPipe();
    bool editStation();
    bool deleteObject();
    bool searchObjects();
    bool saveToFile(const std::string& fileName) const;
    bool loadFromFile(const std::string& fileName);
};
