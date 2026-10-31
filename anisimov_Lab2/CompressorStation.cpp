#include "CompressorStation.h"
#include "Utils.h"

bool CompressorStation::valid() const
{
    return id > 0 && id < std::numeric_limits<int>::max() && validName(name)
        && workshopCount > 0 && workingWorkshops >= 0
        && workingWorkshops <= workshopCount && stationClass > 0;
}

bool CompressorStation::input()
{
    CompressorStation candidate(id);
    if (!readName("Название КС: ", candidate.name)
        || !readNumber("Количество цехов (больше 0): ", candidate.workshopCount, 1, std::numeric_limits<int>::max())
        || !readNumber("Цехов в работе: ", candidate.workingWorkshops, 0, candidate.workshopCount)
        || !readNumber("Класс станции (больше 0): ", candidate.stationClass, 1, std::numeric_limits<int>::max()))
        return false;
    *this = candidate;
    return true;
}

double CompressorStation::getIdlePercent() const
{
    return workshopCount > 0 ? (workshopCount - workingWorkshops) * 100.0 / workshopCount : 0;
}

bool CompressorStation::startWorkshop()
{
    if (workingWorkshops >= workshopCount) return false;
    ++workingWorkshops;
    return true;
}

bool CompressorStation::stopWorkshop()
{
    if (workingWorkshops == 0) return false;
    --workingWorkshops;
    return true;
}

void CompressorStation::show() const
{
    std::cout << "\nКОМПРЕССОРНАЯ СТАНЦИЯ, ID: " << id << "\nНазвание: " << name
        << "\nВсего цехов: " << workshopCount << "\nЦехов в работе: " << workingWorkshops
        << "\nКласс станции: " << stationClass << "\nНезадействованных цехов: " << getIdlePercent() << "%\n";
}

std::ostream& operator<<(std::ostream& out, const CompressorStation& station)
{
    if (!station.valid())
    {
        out.setstate(std::ios::failbit);
        return out;
    }
    return out << station.id << '\n' << station.name << '\n' << station.workshopCount
        << '\n' << station.workingWorkshops << '\n' << station.stationClass << '\n';
}

std::istream& operator>>(std::istream& in, CompressorStation& station)
{
    CompressorStation candidate;
    if (!readNumberLine(in, candidate.id) || !std::getline(in, candidate.name)
        || !readNumberLine(in, candidate.workshopCount) || !readNumberLine(in, candidate.workingWorkshops)
        || !readNumberLine(in, candidate.stationClass) || !candidate.valid())
        in.setstate(std::ios::failbit);
    else station = candidate;
    return in;
}
