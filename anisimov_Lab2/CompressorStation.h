#pragma once
#include <iosfwd>
#include <string>

class CompressorStation
{
    std::string name;
    int workshopCount = 0;
    int workingWorkshops = 0;
    int stationClass = 0;

public:
    const std::string& getName() const { return name; }
    double getIdlePercent() const;
    bool startWorkshop();
    bool stopWorkshop();
    bool input();
    void show(int id) const;
    friend std::ostream& operator<<(std::ostream& out, const CompressorStation& station);
    friend std::istream& operator>>(std::istream& in, CompressorStation& station);
};
