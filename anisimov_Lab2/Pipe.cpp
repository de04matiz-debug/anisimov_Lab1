#include "Pipe.h"
#include "Utils.h"
#include <iomanip>

bool Pipe::valid() const
{
    return id > 0 && id < std::numeric_limits<int>::max() && validName(name)
        && std::isfinite(length) && length > 0 && diameter > 0;
}

bool Pipe::input()
{
    Pipe candidate(id);
    int repair = 0;
    if (!readName("Название трубы: ", candidate.name)
        || !readNumber("Длина (км, больше 0): ", candidate.length,
            std::numeric_limits<double>::denorm_min(), std::numeric_limits<double>::max())
        || !readNumber("Диаметр (мм, больше 0): ", candidate.diameter, 1, std::numeric_limits<int>::max())
        || !readNumber("В ремонте? 0 - нет, 1 - да: ", repair, 0, 1)) return false;
    candidate.inRepair = (repair == 1);
    *this = candidate;
    return true;
}

void Pipe::show() const
{
    std::cout << "\nТРУБА, ID: " << id << "\nНазвание: " << name
        << "\nДлина: " << length << " км\nДиаметр: " << diameter
        << " мм\nВ ремонте: " << (inRepair ? "Да" : "Нет") << '\n';
}

// Потоковые операторы работают с полями; файл открывает Manager.
std::ostream& operator<<(std::ostream& out, const Pipe& pipe)
{
    if (!pipe.valid())
    {
        out.setstate(std::ios::failbit);
        return out;
    }
    const auto precision = out.precision();
    out << pipe.id << '\n' << pipe.name << '\n'
        << std::setprecision(std::numeric_limits<double>::max_digits10)
        << pipe.length << '\n' << pipe.diameter << '\n' << (pipe.inRepair ? 1 : 0) << '\n';
    out.precision(precision);
    return out;
}

std::istream& operator>>(std::istream& in, Pipe& pipe)
{
    Pipe candidate;
    int repair = 0;
    if (!readNumberLine(in, candidate.id) || !std::getline(in, candidate.name)
        || !readNumberLine(in, candidate.length) || !readNumberLine(in, candidate.diameter)
        || !readNumberLine(in, repair) || (repair != 0 && repair != 1) || !candidate.valid())
        in.setstate(std::ios::failbit);
    else
    {
        candidate.inRepair = (repair == 1);
        pipe = candidate;
    }
    return in;
}
