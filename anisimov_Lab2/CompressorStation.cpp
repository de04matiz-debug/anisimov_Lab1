#include "CompressorStation.h"
#include "Utils.h"

bool CompressorStation::input()
{
    return readName("Название КС: ", name)
        && readNumber("Количество цехов (больше 0): ", workshopCount, 1, std::numeric_limits<int>::max())
        && readNumber("Цехов в работе: ", workingWorkshops, 0, workshopCount)
        && readNumber("Класс станции (больше 0): ", stationClass, 1, std::numeric_limits<int>::max());
}

double CompressorStation::getIdlePercent() const
{
    return (workshopCount - workingWorkshops) * 100.0 / workshopCount;
}

bool CompressorStation::startWorkshop()
{
    if (workingWorkshops == workshopCount) return false;
    ++workingWorkshops;
    return true;
}

bool CompressorStation::stopWorkshop()
{
    if (workingWorkshops == 0) return false;
    --workingWorkshops;
    return true;
}

void CompressorStation::show(int id) const
{
    std::cout << "\nКОМПРЕССОРНАЯ СТАНЦИЯ, ID: " << id << "\nНазвание: " << name
        << "\nВсего цехов: " << workshopCount << "\nЦехов в работе: " << workingWorkshops
        << "\nКласс станции: " << stationClass << "\nНезадействованных цехов: " << getIdlePercent() << "%\n";
}

std::ostream& operator<<(std::ostream& out, const CompressorStation& station)
{
    return out << station.name << '\n' << station.workshopCount
        << '\n' << station.workingWorkshops << '\n' << station.stationClass << '\n';
}

std::istream& operator>>(std::istream& in, CompressorStation& station)
{
    if (!std::getline(in, station.name) || !validName(station.name)
        || !readNumberLine(in, station.workshopCount) || station.workshopCount <= 0
        || !readNumberLine(in, station.workingWorkshops) || station.workingWorkshops < 0
        || station.workingWorkshops > station.workshopCount
        || !readNumberLine(in, station.stationClass) || station.stationClass <= 0)
        in.setstate(std::ios::failbit);
    return in;
}
