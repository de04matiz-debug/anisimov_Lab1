#include <iostream>
#include <string>
#include <sstream>
#include <limits>
#include <cmath>
#include <fstream>
#include <filesystem>
#include <iomanip>

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

struct AppState
{
    Pipe pipe;
    CompressorStation station;
    bool hasPipe = false;
    bool hasStation = false;
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
    istringstream row(line);
    T candidate{};
    if (!(row >> candidate) || !isfinite(candidate))
        return false;
    row >> ws;
    if (!row.eof())
        return false;
    value = candidate;
    return true;
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
    Pipe candidate;
    int repair = 0;
    if (!readName("Название трубы (километровая отметка): ", candidate.name)
        || !readNumber("Длина (км, больше 0): ", candidate.length,
            numeric_limits<double>::denorm_min(), numeric_limits<double>::max())
        || !readNumber("Диаметр (мм, больше 0): ", candidate.diameter, 1, numeric_limits<int>::max())
        || !readNumber("В ремонте? 0 - нет, 1 - да: ", repair, 0, 1))
        return false;
    candidate.inRepair = (repair == 1);
    pipe = candidate;
    cout << "Труба добавлена.\n";
    return true;
}

bool addStation(CompressorStation& station)
{
    CompressorStation candidate;
    if (!readName("Название КС: ", candidate.name)
        || !readNumber("Количество цехов (больше 0): ", candidate.workshopCount, 1, numeric_limits<int>::max())
        || !readNumber("Цехов в работе (от 0 до " + to_string(candidate.workshopCount) + "): ", candidate.workingWorkshops, 0, candidate.workshopCount)
        || !readNumber("Класс станции (больше 0): ", candidate.stationClass, 1, numeric_limits<int>::max()))
        return false;
    station = candidate;
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
    if (!validPipe(pipe)) return false;
    out << pipe.name << '\n' << setprecision(numeric_limits<double>::max_digits10)
        << pipe.length << '\n' << pipe.diameter << '\n' << (pipe.inRepair ? 1 : 0) << '\n';
    return static_cast<bool>(out);
}

bool loadPipe(istream& in, Pipe& pipe)
{
    Pipe candidate;
    int repair = 0;
    if (!getline(in, candidate.name)
        || !readNumberLine(in, candidate.length)
        || !readNumberLine(in, candidate.diameter)
        || !readNumberLine(in, repair) || (repair != 0 && repair != 1)
        || !validPipe(candidate))
        return false;
    candidate.inRepair = (repair == 1);
    pipe = candidate;
    return true;
}

bool saveCompressorStation(ostream& out, const CompressorStation& station)
{
    if (!validStation(station)) return false;
    out << station.name << '\n' << station.workshopCount << '\n'
        << station.workingWorkshops << '\n' << station.stationClass << '\n';
    return static_cast<bool>(out);
}

bool loadCompressorStation(istream& in, CompressorStation& station)
{
    CompressorStation candidate;
    if (!getline(in, candidate.name)
        || !readNumberLine(in, candidate.workshopCount)
        || !readNumberLine(in, candidate.workingWorkshops)
        || !readNumberLine(in, candidate.stationClass) || !validStation(candidate))
        return false;
    station = candidate;
    return true;
}

bool saveToFile(const string& fileName, const AppState& state)
{
    error_code error;
    if (!filesystem::is_regular_file(filesystem::u8path(fileName), error))
    {
        cout << "Ошибка сохранения: выберите существующий файл.\n";
        return false;
    }
    if ((state.hasPipe && !validPipe(state.pipe))
        || (state.hasStation && !validStation(state.station)))
    {
        cout << "Ошибка сохранения: неверные данные объекта.\n";
        return false;
    }
    ofstream out(filesystem::u8path(fileName), ios::trunc);
    if (!out)
    {
        cout << "Ошибка сохранения: файл не открывается для записи.\n";
        return false;
    }
    out << (state.hasPipe ? 1 : 0) << '\n';
    bool success = !state.hasPipe || savePipe(out, state.pipe);
    out << (state.hasStation ? 1 : 0) << '\n';
    success = success && (!state.hasStation || saveCompressorStation(out, state.station));
    out.close();
    if (!success || !out)
    {
        cout << "Ошибка записи или закрытия файла.\n";
        return false;
    }
    return true;
}

bool loadFromFile(const string& fileName, AppState& state)
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
    // Меняем данные только после проверки всего файла.
    AppState candidate;
    int pipeCount = 0, stationCount = 0;
    if (!readNumberLine(in, pipeCount) || (pipeCount != 0 && pipeCount != 1))
    {
        cout << "Ошибка загрузки: количество труб должно быть 0 или 1.\n";
        return false;
    }
    candidate.hasPipe = (pipeCount == 1);
    if (candidate.hasPipe && !loadPipe(in, candidate.pipe))
    {
        cout << "Ошибка загрузки: неверные или неполные поля трубы.\n";
        return false;
    }
    if (!readNumberLine(in, stationCount) || (stationCount != 0 && stationCount != 1))
    {
        cout << "Ошибка загрузки: количество КС должно быть 0 или 1.\n";
        return false;
    }
    candidate.hasStation = (stationCount == 1);
    if (candidate.hasStation && !loadCompressorStation(in, candidate.station))
    {
        cout << "Ошибка загрузки: неверные или неполные поля КС.\n";
        return false;
    }
    string extraLine;
    if (getline(in, extraLine) || in.bad() || !in.eof())
    {
        cout << "Ошибка загрузки: лишние поля или ошибка чтения.\n";
        return false;
    }
    state = candidate;
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
    AppState state;
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
            if (!addPipe(state.pipe)) return 0;
            state.hasPipe = true;
            break;
        case 2:
            if (!addStation(state.station)) return 0;
            state.hasStation = true;
            break;
        case 3:
            if (state.hasPipe) showPipe(state.pipe);
            else cout << "Труба не добавлена.\n";
            if (state.hasStation) showStation(state.station);
            else cout << "КС не добавлена.\n";
            break;
        case 4:
            if (!state.hasPipe) cout << "Сначала добавьте трубу.\n";
            else if (!editPipe(state.pipe)) return 0;
            break;
        case 5:
            if (!state.hasStation) cout << "Сначала добавьте КС.\n";
            else if (!editStation(state.station)) return 0;
            break;
        case 6:
        case 7:
            if (!readName("Путь к существующему файлу: ", fileName)) return 0;
            if (choice == 6)
            {
                if (saveToFile(fileName, state)) cout << "Данные сохранены.\n";
            }
            else if (loadFromFile(fileName, state)) cout << "Данные загружены.\n";
            break;
        }
    }
    cout << "Программа завершена.\n";
    return 0;
}
