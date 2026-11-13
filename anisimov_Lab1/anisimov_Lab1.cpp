#include <iostream>
#include <string>
#include <charconv>
#include <limits>
#include <cmath>
#include <fstream>
#include <filesystem>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

using namespace std;

struct Pipe
{
    string name;
    double length = 0;
    int diameter = 0;
    bool inRepair = false;
};

struct CompressorStation
{
    string name;
    int workshopCount = 0;
    int workingWorkshops = 0;
    int stationClass = 0;
};

bool validName(const string& name)
{
    return name.find_first_not_of(" \t\r\n") != string::npos
        && name.find_first_of("\r\n") == string::npos;
}

// Проверяем всю строку, чтобы не принимать число с лишним текстом.
template <typename T>
bool readNumberLine(istream& in, T& value)
{
    string line;
    if (!getline(in, line))
        return false;
    size_t first = line.find_first_not_of(" \t\r\n\v\f");
    if (first == string::npos)
        return false;
    size_t last = line.find_last_not_of(" \t\r\n\v\f") + 1;
    if (line[first] == '+')
    {
        ++first;
        if (first == last || line[first] == '-' || line[first] == '+')
            return false;
    }
    auto result = from_chars(line.data() + first, line.data() + last, value);
    return result.ec == errc{} && result.ptr == line.data() + last && isfinite(value);
}

template <typename T>
bool readNumber(const string& prompt, T& value, T minimum, T maximum)
{
    while (true)
    {
        cout << prompt;
        if (readNumberLine(cin, value) && value >= minimum && value <= maximum)
            return true;
        if (!cin)
            return false;
        cout << "Ошибка ввода. Повторите ввод.\n";
    }
}

bool readName(const string& prompt, string& name)
{
    while (true)
    {
        cout << prompt;
        if (!getline(cin, name))
            return false;
        if (validName(name))
            return true;
        cout << "Название не должно быть пустым.\n";
    }
}

bool addPipe(Pipe& pipe)
{
    int repair = 0;
    if (!readName("Название трубы (километровая отметка): ", pipe.name)
        || !readNumber("Длина (км, больше 0): ", pipe.length,
            numeric_limits<double>::denorm_min(), numeric_limits<double>::max())
        || !readNumber("Диаметр (мм, больше 0): ", pipe.diameter, 1, numeric_limits<int>::max())
        || !readNumber("В ремонте? 0 - нет, 1 - да: ", repair, 0, 1))
        return false;
    pipe.inRepair = (repair == 1);
    cout << "Труба добавлена.\n";
    return true;
}

bool addStation(CompressorStation& station)
{
    if (!readName("Название КС: ", station.name)
        || !readNumber("Количество цехов (больше 0): ", station.workshopCount, 1, numeric_limits<int>::max())
        || !readNumber("Цехов в работе (от 0 до " + to_string(station.workshopCount) + "): ", station.workingWorkshops, 0, station.workshopCount)
        || !readNumber("Класс станции (больше 0): ", station.stationClass, 1, numeric_limits<int>::max()))
        return false;
    cout << "КС добавлена.\n";
    return true;
}

void showPipe(const Pipe& pipe)
{
    cout << "\nТРУБА\nНазвание: " << pipe.name
        << "\nДлина: " << pipe.length << " км\nДиаметр: " << pipe.diameter
        << " мм\nВ ремонте: " << (pipe.inRepair ? "Да" : "Нет") << '\n';
}

void showStation(const CompressorStation& station)
{
    cout << "\nКОМПРЕССОРНАЯ СТАНЦИЯ\nНазвание: " << station.name
        << "\nВсего цехов: " << station.workshopCount
        << "\nЦехов в работе: " << station.workingWorkshops
        << "\nКласс станции: " << station.stationClass << '\n';
}

bool editPipe(Pipe& pipe)
{
    int repair = 0;
    if (!readNumber("В ремонте? 0 - нет, 1 - да: ", repair, 0, 1))
        return false;
    pipe.inRepair = (repair == 1);
    cout << "Состояние трубы изменено.\n";
    return true;
}

bool editStation(CompressorStation& station)
{
    int action = 0;
    if (!readNumber("1 - запустить цех, 2 - остановить цех, 0 - отмена: ", action, 0, 2))
        return false;
    if (action == 1)
    {
        if (station.workingWorkshops == station.workshopCount)
            cout << "Все цехи уже работают.\n";
        else
        {
            ++station.workingWorkshops;
            cout << "Цех запущен.\n";
        }
    }
    else if (action == 2)
    {
        if (station.workingWorkshops == 0)
            cout << "Нет работающих цехов.\n";
        else
        {
            --station.workingWorkshops;
            cout << "Цех остановлен.\n";
        }
    }
    return true;
}

bool validPipe(const Pipe& pipe)
{
    return validName(pipe.name) && isfinite(pipe.length) && pipe.length > 0
        && pipe.diameter > 0;
}

bool validStation(const CompressorStation& station)
{
    return validName(station.name) && station.workshopCount > 0
        && station.workingWorkshops >= 0
        && station.workingWorkshops <= station.workshopCount && station.stationClass > 0;
}

// Эти функции сохраняют и загружают только поля объекта.
bool savePipe(ostream& out, const Pipe& pipe)
{
    out.precision(numeric_limits<double>::max_digits10);
    out << pipe.name << '\n' << pipe.length << '\n'
        << pipe.diameter << '\n' << (pipe.inRepair ? 1 : 0) << '\n';
    return static_cast<bool>(out);
}

bool loadPipe(istream& in, Pipe& pipe)
{
    int repair = 0;
    if (!getline(in, pipe.name)
        || !readNumberLine(in, pipe.length)
        || !readNumberLine(in, pipe.diameter)
        || !readNumberLine(in, repair) || (repair != 0 && repair != 1)
        || !validPipe(pipe))
        return false;
    pipe.inRepair = (repair == 1);
    return true;
}

bool saveCompressorStation(ostream& out, const CompressorStation& station)
{
    out << station.name << '\n' << station.workshopCount << '\n'
        << station.workingWorkshops << '\n' << station.stationClass << '\n';
    return static_cast<bool>(out);
}

bool loadCompressorStation(istream& in, CompressorStation& station)
{
    return getline(in, station.name)
        && readNumberLine(in, station.workshopCount)
        && readNumberLine(in, station.workingWorkshops)
        && readNumberLine(in, station.stationClass) && validStation(station);
}

bool saveToFile(const string& fileName, const Pipe& pipe, const CompressorStation& station)
{
    error_code error;
    if (!filesystem::is_regular_file(filesystem::u8path(fileName), error))
    {
        cout << "Ошибка сохранения: выберите существующий файл.\n";
        return false;
    }
    ofstream out(filesystem::u8path(fileName), ios::trunc);
    if (!out)
    {
        cout << "Ошибка сохранения: файл не открывается для записи.\n";
        return false;
    }
    out << (pipe.name.empty() ? 0 : 1) << '\n';
    bool success = pipe.name.empty() || savePipe(out, pipe);
    out << (station.name.empty() ? 0 : 1) << '\n';
    success = success && (station.name.empty() || saveCompressorStation(out, station));
    out.close();
    if (!success || !out)
    {
        cout << "Ошибка записи или закрытия файла.\n";
        return false;
    }
    return true;
}

bool loadFromFile(const string& fileName, Pipe& pipe, CompressorStation& station)
{
    error_code error;
    if (!filesystem::is_regular_file(filesystem::u8path(fileName), error))
    {
        cout << "Ошибка загрузки: выберите существующий файл.\n";
        return false;
    }
    ifstream in(filesystem::u8path(fileName));
    if (!in)
    {
        cout << "Ошибка загрузки: файл не открывается для чтения.\n";
        return false;
    }
    pipe = {};
    station = {};
    int pipeCount = 0, stationCount = 0;
    bool success = readNumberLine(in, pipeCount) && (pipeCount == 0 || pipeCount == 1);
    if (success && pipeCount == 1)
        success = loadPipe(in, pipe);
    if (success)
        success = readNumberLine(in, stationCount) && (stationCount == 0 || stationCount == 1);
    if (success && stationCount == 1)
        success = loadCompressorStation(in, station);
    string extraLine;
    if (!success || getline(in, extraLine) || in.bad() || !in.eof())
    {
        // Частично прочитанные объекты не должны попасть в меню и сохранение.
        pipe = {};
        station = {};
        cout << "Ошибка загрузки: неверные или неполные данные. Объекты очищены.\n";
        return false;
    }
    return true;
}

void showMenu()
{
    cout << "\n1. Добавить трубу\n2. Добавить КС\n3. Просмотр объектов\n"
        << "4. Изменить ремонт трубы\n5. Запуск или остановка цеха КС\n6. Сохранить\n7. Загрузить\n0. Выход\n";
}

int main()
{
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif
    Pipe pipe;
    CompressorStation station;
    int choice = 0;
    string fileName;
    while (true)
    {
        showMenu();
        if (!readNumber("Выберите действие: ", choice, 0, 7) || choice == 0)
            break;
        switch (choice)
        {
        case 1:
            if (!addPipe(pipe)) return 0;
            break;
        case 2:
            if (!addStation(station)) return 0;
            break;
        case 3:
            if (!pipe.name.empty()) showPipe(pipe);
            else cout << "Труба не добавлена.\n";
            if (!station.name.empty()) showStation(station);
            else cout << "КС не добавлена.\n";
            break;
        case 4:
            if (pipe.name.empty()) cout << "Сначала добавьте трубу.\n";
            else if (!editPipe(pipe)) return 0;
            break;
        case 5:
            if (station.name.empty()) cout << "Сначала добавьте КС.\n";
            else if (!editStation(station)) return 0;
            break;
        case 6:
        case 7:
            if (!readName("Путь к существующему файлу: ", fileName)) return 0;
            if (choice == 6)
            {
                if (saveToFile(fileName, pipe, station)) cout << "Данные сохранены.\n";
            }
            else if (loadFromFile(fileName, pipe, station)) cout << "Данные загружены.\n";
            break;
        }
    }
    cout << "Программа завершена.\n";
    return 0;
}
