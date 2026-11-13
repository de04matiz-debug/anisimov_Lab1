#include "Pipe.h"
#include "Utils.h"

bool Pipe::input()
{
    int repair = 0;
    if (!readName("Название трубы: ", name)
        || !readNumber("Длина (км, больше 0): ", length,
            std::numeric_limits<double>::denorm_min(), std::numeric_limits<double>::max())
        || !readNumber("Диаметр (мм, больше 0): ", diameter, 1, std::numeric_limits<int>::max())
        || !readNumber("В ремонте? 0 - нет, 1 - да: ", repair, 0, 1)) return false;
    inRepair = (repair == 1);
    return true;
}

void Pipe::show(int id) const
{
    std::cout << "\nТРУБА, ID: " << id << "\nНазвание: " << name
        << "\nДлина: " << length << " км\nДиаметр: " << diameter
        << " мм\nВ ремонте: " << (inRepair ? "Да" : "Нет") << '\n';
}

std::ostream& operator<<(std::ostream& out, const Pipe& pipe)
{
    return out << pipe.name << '\n' << pipe.length << '\n'
        << pipe.diameter << '\n' << pipe.inRepair << '\n';
}

std::istream& operator>>(std::istream& in, Pipe& pipe)
{
    int repair = 0;
    if (!std::getline(in, pipe.name) || !validName(pipe.name)
        || !readNumberLine(in, pipe.length) || pipe.length <= 0
        || !readNumberLine(in, pipe.diameter) || pipe.diameter <= 0
        || !readNumberLine(in, repair) || (repair != 0 && repair != 1))
        in.setstate(std::ios::failbit);
    else pipe.inRepair = (repair == 1);
    return in;
}
