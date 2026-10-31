#pragma once
#include <iosfwd>
#include <string>

class CompressorStation
{
    int id = 0;
    std::string name;
    int workshopCount = 0;
    int workingWorkshops = 0;
    int stationClass = 0;

public:
    explicit CompressorStation(int objectId = 0) : id(objectId) {}
    int getId() const { return id; }
    const std::string& getName() const { return name; }
    double getIdlePercent() const;
    bool startWorkshop();
    bool stopWorkshop();
    bool valid() const;
    bool input();
    void show() const;
    friend std::ostream& operator<<(std::ostream& out, const CompressorStation& station);
    friend std::istream& operator>>(std::istream& in, CompressorStation& station);
};
