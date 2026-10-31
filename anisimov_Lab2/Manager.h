#pragma once
#include "Pipe.h"
#include "CompressorStation.h"
#include <map>
#include <string>

struct AppState
{
    std::map<int, Pipe> pipes;
    std::map<int, CompressorStation> stations;
    int nextPipeId = 1;
    int nextStationId = 1;
};

void showObjects(const AppState& state);
bool addPipe(AppState& state);
bool addStation(AppState& state);
bool editPipe(AppState& state);
bool editStation(AppState& state);
bool deleteObject(AppState& state);
bool searchObjects(AppState& state);
bool saveToFile(const std::string& fileName, const AppState& state);
bool loadFromFile(const std::string& fileName, AppState& state);
