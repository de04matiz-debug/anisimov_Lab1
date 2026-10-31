#include "Manager.h"
#include "Utils.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <set>

using namespace std;

namespace
{
    void report(const string& message)
    {
        cout << message << '\n';
        logAction(message);
    }

    template <typename T>
    bool chooseId(const map<int, T>& objects, int& id)
    {
        while (readNumber("ID объекта (0 - отмена): ", id, 0, numeric_limits<int>::max() - 1))
        {
            if (id == 0 || objects.count(id) != 0) return true;
            report("Объект с таким ID не найден.");
        }
        return false;
    }

    bool selectPipes(const vector<int>& found, vector<int>& selected)
    {
        int mode = 0;
        if (!readNumber("1 - все найденные, 2 - выбрать ID, 0 - отмена: ", mode, 0, 2)) return false;
        if (mode == 0) return true;
        if (mode == 1)
        {
            selected = found;
            return true;
        }
        string line;
        while (readLine("ID из найденных через пробел (0 - отмена): ", line))
        {
            if (line == "0") return true;
            istringstream row(line);
            set<int> ids;
            string token;
            bool correct = true;
            while (row >> token)
            {
                int id = 0;
                istringstream number(token);
                if (!readNumberLine(number, id) || find(found.begin(), found.end(), id) == found.end()
                    || !ids.insert(id).second)
                {
                    correct = false;
                    break;
                }
            }
            if (correct && !ids.empty())
            {
                selected.assign(ids.begin(), ids.end());
                return true;
            }
            report("Ошибка: укажите неповторяющиеся ID только из найденных труб.");
        }
        return false;
    }

    bool managePipes(AppState& state, const vector<int>& found)
    {
        int action = 0;
        if (!readNumber("1 - в ремонт, 2 - из ремонта, 3 - удалить, 0 - назад: ", action, 0, 3)) return false;
        if (action == 0) return true;
        vector<int> selected;
        if (!selectPipes(found, selected)) return false;
        if (selected.empty())
        {
            report("Пакетная операция отменена.");
            return true;
        }
        for (int id : selected)
        {
            if (action == 3)
            {
                state.pipes.erase(id);
                logAction("Удалена труба ID=" + to_string(id));
            }
            else
            {
                auto& pipe = state.pipes.at(id);
                logAction("Ремонт трубы ID=" + to_string(id) + ": "
                    + to_string(pipe.isRepairing()) + " -> " + to_string(action == 1));
                pipe.setRepairing(action == 1);
            }
        }
        report("Обработано труб: " + to_string(selected.size()));
        return true;
    }

    bool searchPipes(AppState& state)
    {
        if (state.pipes.empty()) { report("Трубы не добавлены."); return true; }
        int filter = 0;
        if (!readNumber("Поиск труб: 1 - по названию, 2 - по ремонту, 0 - назад: ", filter, 0, 2)) return false;
        if (filter == 0) return true;
        vector<int> found;
        if (filter == 1)
        {
            string query;
            if (!readName("Часть названия (с учётом регистра): ", query)) return false;
            found = findIds(state.pipes, [&query](const Pipe& p) {
                return p.getName().find(query) != string::npos;
            });
        }
        else
        {
            int repair = 0;
            if (!readNumber("В ремонте? 0 - нет, 1 - да: ", repair, 0, 1)) return false;
            found = findIds(state.pipes, [repair](const Pipe& p) { return p.isRepairing() == (repair == 1); });
        }
        report("Найдено труб: " + to_string(found.size()));
        for (int id : found)
        {
            state.pipes.at(id).show();
            logAction("Найдена труба ID=" + to_string(id));
        }
        return found.empty() || managePipes(state, found);
    }

    bool searchStations(const AppState& state)
    {
        if (state.stations.empty()) { report("КС не добавлены."); return true; }
        int filter = 0;
        if (!readNumber("Поиск КС: 1 - по названию, 2 - по проценту незадействованных цехов, 0 - назад: ", filter, 0, 2)) return false;
        if (filter == 0) return true;
        vector<int> found;
        if (filter == 1)
        {
            string query;
            if (!readName("Часть названия (с учётом регистра): ", query)) return false;
            found = findIds(state.stations, [&query](const CompressorStation& s) {
                return s.getName().find(query) != string::npos;
            });
        }
        else
        {
            double percent = 0;
            if (!readNumber("Минимальный процент незадействованных цехов (0-100): ", percent, 0.0, 100.0)) return false;
            found = findIds(state.stations, [percent](const CompressorStation& s) {
                return s.getIdlePercent() >= percent;
            });
        }
        report("Найдено КС: " + to_string(found.size()));
        for (int id : found)
        {
            state.stations.at(id).show();
            logAction("Найдена КС ID=" + to_string(id));
        }
        return true;
    }

    bool fileError(const string& message)
    {
        report(message);
        return false;
    }
}

void showObjects(const AppState& state)
{
    if (state.pipes.empty() && state.stations.empty()) cout << "Объекты не добавлены.\n";
    for (const auto& [id, pipe] : state.pipes) pipe.show();
    for (const auto& [id, station] : state.stations) station.show();
    logAction("Просмотр: труб=" + to_string(state.pipes.size()) + ", КС=" + to_string(state.stations.size()));
}

bool addPipe(AppState& state)
{
    if (state.nextPipeId == numeric_limits<int>::max()) { report("Закончились ID труб."); return true; }
    Pipe pipe(state.nextPipeId);
    if (!pipe.input()) return false;
    state.pipes.emplace(pipe.getId(), pipe);
    ++state.nextPipeId;
    report("Добавлена труба, ID: " + to_string(pipe.getId()));
    return true;
}

bool addStation(AppState& state)
{
    if (state.nextStationId == numeric_limits<int>::max()) { report("Закончились ID КС."); return true; }
    CompressorStation station(state.nextStationId);
    if (!station.input()) return false;
    state.stations.emplace(station.getId(), station);
    ++state.nextStationId;
    report("Добавлена КС, ID: " + to_string(station.getId()));
    return true;
}

bool editPipe(AppState& state)
{
    if (state.pipes.empty()) { report("Трубы не добавлены."); return true; }
    int id = 0, repair = 0;
    if (!chooseId(state.pipes, id)) return false;
    if (id == 0) { report("Редактирование отменено."); return true; }
    if (!readNumber("В ремонте? 0 - нет, 1 - да: ", repair, 0, 1)) return false;
    auto& pipe = state.pipes.at(id);
    logAction("Ремонт трубы ID=" + to_string(id) + ": " + to_string(pipe.isRepairing()) + " -> " + to_string(repair));
    pipe.setRepairing(repair == 1);
    report("Состояние трубы изменено.");
    return true;
}

bool editStation(AppState& state)
{
    if (state.stations.empty()) { report("КС не добавлены."); return true; }
    int id = 0, action = 0;
    if (!chooseId(state.stations, id)) return false;
    if (id == 0) { report("Редактирование отменено."); return true; }
    if (!readNumber("1 - запустить цех, 2 - остановить цех, 0 - отмена: ", action, 0, 2)) return false;
    if (action == 0) { report("Редактирование отменено."); return true; }
    auto& station = state.stations.at(id);
    logAction("Изменение КС ID=" + to_string(id));
    if (action == 1) report(station.startWorkshop() ? "Цех запущен." : "Все цехи уже работают.");
    else report(station.stopWorkshop() ? "Цех остановлен." : "Нет работающих цехов.");
    return true;
}

bool deleteObject(AppState& state)
{
    int type = 0, id = 0;
    if (!readNumber("Удалить: 1 - трубу, 2 - КС, 0 - отмена: ", type, 0, 2)) return false;
    if (type == 0) { report("Удаление отменено."); return true; }
    if (type == 1)
    {
        if (state.pipes.empty()) { report("Трубы не добавлены."); return true; }
        if (!chooseId(state.pipes, id)) return false;
        if (id != 0) { state.pipes.erase(id); report("Удалена труба ID=" + to_string(id)); }
    }
    else
    {
        if (state.stations.empty()) { report("КС не добавлены."); return true; }
        if (!chooseId(state.stations, id)) return false;
        if (id != 0) { state.stations.erase(id); report("Удалена КС ID=" + to_string(id)); }
    }
    if (id == 0) report("Удаление отменено.");
    return true;
}

bool searchObjects(AppState& state)
{
    int type = 0;
    if (!readNumber("Поиск: 1 - трубы, 2 - КС, 0 - назад: ", type, 0, 2)) return false;
    if (type == 1) return searchPipes(state);
    if (type == 2) return searchStations(state);
    logAction("Поиск отменён");
    return true;
}

bool saveToFile(const string& fileName, const AppState& state)
{
    logAction("Сохранение в файл: " + fileName);
    const auto path = filesystem::u8path(fileName);
    error_code error;
    if (!filesystem::is_regular_file(path, error))
        return fileError("Ошибка сохранения: выберите существующий файл.");
    // Не разрешаем перезаписать журнал текущего запуска.
    if (filesystem::equivalent(path, filesystem::u8path("log.txt"), error))
        return fileError("Ошибка сохранения: нельзя перезаписать журнал.");
    for (const auto& [id, pipe] : state.pipes)
        if (!pipe.valid() || id != pipe.getId()) return fileError("Ошибка: неверные данные трубы.");
    for (const auto& [id, station] : state.stations)
        if (!station.valid() || id != station.getId()) return fileError("Ошибка: неверные данные КС.");
    ofstream out(path, ios::trunc);
    if (!out) return fileError("Ошибка сохранения: файл не открывается для записи.");
    out << state.pipes.size() << '\n';
    for (const auto& [id, pipe] : state.pipes) out << pipe;
    out << state.stations.size() << '\n';
    for (const auto& [id, station] : state.stations) out << station;
    out.close();
    if (!out) return fileError("Ошибка записи или закрытия файла.");
    report("Данные сохранены.");
    return true;
}

bool loadFromFile(const string& fileName, AppState& state)
{
    logAction("Загрузка из файла: " + fileName);
    ifstream in(filesystem::u8path(fileName));
    if (!in) return fileError("Ошибка загрузки: файл не открывается для чтения.");
    // Текущие коллекции и счётчики меняются только после проверки всего файла.
    AppState candidate;
    candidate.nextPipeId = state.nextPipeId;
    candidate.nextStationId = state.nextStationId;
    int count = 0;
    if (!readNumberLine(in, count) || count < 0) return fileError("Ошибка загрузки: неверное количество труб.");
    for (int i = 0; i < count; ++i)
    {
        Pipe pipe;
        if (!(in >> pipe) || !candidate.pipes.emplace(pipe.getId(), pipe).second)
            return fileError("Ошибка загрузки: неверная труба или повторяющийся ID.");
        candidate.nextPipeId = max(candidate.nextPipeId, pipe.getId() + 1);
    }
    if (!readNumberLine(in, count) || count < 0) return fileError("Ошибка загрузки: неверное количество КС.");
    for (int i = 0; i < count; ++i)
    {
        CompressorStation station;
        if (!(in >> station) || !candidate.stations.emplace(station.getId(), station).second)
            return fileError("Ошибка загрузки: неверная КС или повторяющийся ID.");
        candidate.nextStationId = max(candidate.nextStationId, station.getId() + 1);
    }
    string extra;
    if (getline(in, extra) || in.bad() || !in.eof())
        return fileError("Ошибка загрузки: лишние поля или ошибка чтения.");
    state = std::move(candidate);
    report("Данные загружены. Труб: " + to_string(state.pipes.size()) + ", КС: " + to_string(state.stations.size()));
    return true;
}
