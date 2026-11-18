#include "Manager.h"
#include "Utils.h"
#include <algorithm>
#include <filesystem>
#include <fstream>

using namespace std;

namespace
{
    template <typename T>
    bool chooseId(const unordered_map<int, T>& objects, int& id,
        const string& prompt = "ID объекта (0 - отмена): ")
    {
        while (readNumber(prompt, id, 0, numeric_limits<int>::max() - 1))
        {
            if (id == 0 || objects.count(id)) return true;
            report("Объект с таким ID не найден.");
        }
        return false;
    }

    bool selectIds(vector<int>& ids)
    {
        int mode = 0;
        if (!readNumber("1 - все найденные, 2 - выбрать ID, 0 - отмена: ", mode, 0, 2)) return false;
        if (mode == 1) return true;
        vector<int> selected;
        if (mode == 2)
        {
            int id = 0;
            while (true)
            {
                if (!readNumber("ID из найденных (0 - закончить выбор): ", id, 0, numeric_limits<int>::max() - 1)) return false;
                if (id == 0) break;
                if (find(ids.begin(), ids.end(), id) == ids.end())
                    report("Такого ID нет среди найденных труб.");
                else if (find(selected.begin(), selected.end(), id) != selected.end())
                    report("Этот ID уже выбран.");
                else selected.push_back(id);
            }
        }
        ids.swap(selected);
        return true;
    }

    template <typename T>
    bool addObject(unordered_map<int, T>& objects, int& nextId)
    {
        if (nextId == numeric_limits<int>::max())
        {
            report("Закончились свободные ID.");
            return true;
        }
        auto it = objects.try_emplace(nextId).first;
        if (!it->second.input())
        {
            objects.erase(it);
            return false;
        }
        report("Объект добавлен, ID: " + to_string(nextId++));
        return true;
    }

    template <typename T>
    bool readObjects(istream& in, unordered_map<int, T>& objects, int& nextId)
    {
        int count = 0;
        if (!readNumberLine(in, count) || count < 0) return false;
        for (int i = 0; i < count; ++i)
        {
            int id = 0;
            if (!readNumberLine(in, id) || id <= 0 || id == numeric_limits<int>::max()) return false;
            auto [it, inserted] = objects.try_emplace(id);
            if (!inserted || !(in >> it->second)) return false;
            nextId = max(nextId, id + 1);
        }
        return true;
    }
}

void Manager::showObjects() const
{
    if (pipes.empty() && stations.empty()) cout << "Объекты не добавлены.\n";
    for (const auto& [id, pipe] : pipes) pipe.show(id);
    for (const auto& [id, station] : stations) station.show(id);
}

bool Manager::addPipe()
{
    return addObject(pipes, nextPipeId);
}

bool Manager::addStation()
{
    return addObject(stations, nextStationId);
}

bool Manager::editPipe()
{
    if (pipes.empty()) { report("Трубы не добавлены."); return true; }
    int id = 0, repair = 0;
    if (!chooseId(pipes, id)) return false;
    if (id == 0) { report("Редактирование отменено."); return true; }
    if (!readNumber("В ремонте? 0 - нет, 1 - да: ", repair, 0, 1)) return false;
    pipes.at(id).setRepairing(repair == 1);
    report("Ремонт трубы ID=" + to_string(id) + ": " + to_string(repair));
    return true;
}

bool Manager::editStation()
{
    if (stations.empty()) { report("КС не добавлены."); return true; }
    int id = 0, action = 0;
    if (!chooseId(stations, id)) return false;
    if (id == 0) { report("Редактирование отменено."); return true; }
    if (!readNumber("1 - запустить цех, 2 - остановить цех, 0 - отмена: ", action, 0, 2)) return false;
    if (action == 0) { report("Редактирование отменено."); return true; }
    auto& station = stations.at(id);
    if (action == 1) report(station.startWorkshop() ? "Цех запущен." : "Все цехи уже работают.");
    else report(station.stopWorkshop() ? "Цех остановлен." : "Нет работающих цехов.");
    return true;
}

bool Manager::deleteObject()
{
    int type = 0, id = 0;
    if (!readNumber("Удалить: 1 - трубу, 2 - КС, 0 - отмена: ", type, 0, 2)) return false;
    if (type == 0) { report("Удаление отменено."); return true; }
    if (type == 1)
    {
        if (pipes.empty()) { report("Трубы не добавлены."); return true; }
        if (!chooseId(pipes, id)) return false;
        if (id != 0)
        {
            network.disconnectPipe(id);
            pipes.erase(id);
            report("Удалена труба ID=" + to_string(id));
        }
    }
    else
    {
        if (stations.empty()) { report("КС не добавлены."); return true; }
        if (!chooseId(stations, id)) return false;
        if (id != 0)
        {
            network.disconnectStation(id);
            stations.erase(id);
            report("Удалена КС ID=" + to_string(id));
        }
    }
    if (id == 0) report("Удаление отменено.");
    return true;
}

bool Manager::managePipes(vector<int>& found)
{
    int action = 0;
    if (!readNumber("1 - в ремонт, 2 - из ремонта, 3 - удалить, 0 - назад: ", action, 0, 3)) return false;
    if (action == 0) return true;
    if (!selectIds(found)) return false;
    if (found.empty()) { report("Пакетная операция отменена."); return true; }
    for (int id : found)
    {
        if (action == 3)
        {
            network.disconnectPipe(id);
            pipes.erase(id);
        }
        else
        {
            pipes.at(id).setRepairing(action == 1);
        }
    }
    report("Обработано труб: " + to_string(found.size()));
    return true;
}

bool Manager::searchPipes()
{
    if (pipes.empty()) { report("Трубы не добавлены."); return true; }
    int filter = 0;
    if (!readNumber("Поиск труб: 1 - по названию, 2 - по ремонту, 0 - назад: ", filter, 0, 2)) return false;
    if (filter == 0) return true;
    vector<int> found;
    if (filter == 1)
    {
        string query;
        if (!readName("Часть названия (с учётом регистра): ", query)) return false;
        found = findIds(pipes, [&query](const Pipe& p) { return p.getName().find(query) != string::npos; });
    }
    else
    {
        int repair = 0;
        if (!readNumber("В ремонте? 0 - нет, 1 - да: ", repair, 0, 1)) return false;
        found = findIds(pipes, [repair](const Pipe& p) { return p.isRepairing() == (repair == 1); });
    }
    report("Найдено труб: " + to_string(found.size()));
    for (int id : found)
    {
        pipes.at(id).show(id);
    }
    return found.empty() || managePipes(found);
}

bool Manager::searchStations() const
{
    if (stations.empty()) { report("КС не добавлены."); return true; }
    int filter = 0;
    if (!readNumber("Поиск КС: 1 - по названию, 2 - по проценту незадействованных цехов, 0 - назад: ", filter, 0, 2)) return false;
    if (filter == 0) return true;
    vector<int> found;
    if (filter == 1)
    {
        string query;
        if (!readName("Часть названия (с учётом регистра): ", query)) return false;
        found = findIds(stations, [&query](const CompressorStation& s) { return s.getName().find(query) != string::npos; });
    }
    else
    {
        double percent = 0;
        if (!readNumber("Минимальный процент незадействованных цехов (0-100): ", percent, 0.0, 100.0)) return false;
        found = findIds(stations, [percent](const CompressorStation& s) { return s.getIdlePercent() >= percent; });
    }
    report("Найдено КС: " + to_string(found.size()));
    for (int id : found)
    {
        stations.at(id).show(id);
    }
    return true;
}

bool Manager::searchObjects()
{
    int type = 0;
    if (!readNumber("Поиск: 1 - трубы, 2 - КС, 0 - назад: ", type, 0, 2)) return false;
    if (type == 1) return searchPipes();
    if (type == 2) return searchStations();
    return true;
}

bool Manager::saveToFile(const string& fileName) const
{
    const auto path = filesystem::u8path(fileName);
    error_code error;
    if (filesystem::equivalent(path, filesystem::u8path("log.txt"), error))
    {
        report("Ошибка сохранения: нельзя перезаписать журнал.");
        return false;
    }
    ofstream out(path);
    if (!out) { report("Ошибка сохранения: файл не открывается для записи."); return false; }
    out.precision(numeric_limits<double>::max_digits10);
    out << pipes.size() << '\n';
    for (const auto& [id, pipe] : pipes) out << id << '\n' << pipe;
    out << stations.size() << '\n';
    for (const auto& [id, station] : stations) out << id << '\n' << station;
    out << "NETWORK_V1\n" << network.getConnections().size() << '\n';
    for (const auto& [pipeId, connection] : network.getConnections())
        out << pipeId << '\n' << connection.inletId << '\n' << connection.outletId << '\n';
    out.close();
    if (!out) { report("Ошибка записи или закрытия файла."); return false; }
    report("Данные сохранены.");
    return true;
}

bool Manager::loadFromFile(const string& fileName)
{
    ifstream in(filesystem::u8path(fileName));
    if (!in) { report("Ошибка загрузки: файл не открывается для чтения."); return false; }
    pipes.clear();
    stations.clear();
    network.clear();
    if (!readObjects(in, pipes, nextPipeId) || !readObjects(in, stations, nextStationId)
        || !readNetwork(in))
    {
        pipes.clear();
        stations.clear();
        network.clear();
        report("Ошибка загрузки: неверные или неполные данные. Объекты и сеть очищены.");
        return false;
    }
    report("Данные загружены. Труб: " + to_string(pipes.size()) + ", КС: " + to_string(stations.size()));
    return true;
}

bool Manager::readNetwork(istream& in)
{
    string header;
    // У файла ЛР2 после объектов нет раздела сети.
    if (!getline(in, header)) return in.eof() && !in.bad();
    if (header != "NETWORK_V1") return false;
    int count = 0;
    if (!readNumberLine(in, count) || count < 0
        || static_cast<size_t>(count) > pipes.size()) return false;
    for (int i = 0; i < count; ++i)
    {
        int pipeId = 0, inletId = 0, outletId = 0;
        if (!readNumberLine(in, pipeId) || !readNumberLine(in, inletId)
            || !readNumberLine(in, outletId) || !pipes.count(pipeId)
            || !stations.count(inletId) || !stations.count(outletId)
            || !network.connect(pipeId, inletId, outletId)) return false;
    }
    string extra;
    return !getline(in, extra) && in.eof() && !in.bad();
}

bool Manager::connectStations()
{
    if (stations.size() < 2)
    {
        report("Для соединения нужны хотя бы две КС.");
        return true;
    }
    int inletId = 0, outletId = 0, diameter = 0;
    if (!chooseId(stations, inletId, "ID КС входа (откуда газ, 0 - отмена): ")) return false;
    if (inletId == 0) return true;
    while (true)
    {
        if (!chooseId(stations, outletId, "ID КС выхода (куда газ, 0 - отмена): ")) return false;
        if (outletId == 0) return true;
        if (inletId != outletId) break;
        report("Нельзя соединить КС с самой собой. Выберите другую КС.");
    }
    if (!readDiameter("Диаметр трубы (500, 700, 1000, 1400 мм): ", diameter)) return false;

    int pipeId = 0;
    for (const auto& [id, pipe] : pipes)
        if (pipe.getDiameter() == diameter && !network.isPipeConnected(id)
            && (pipeId == 0 || id < pipeId)) pipeId = id;
    if (pipeId == 0)
    {
        if (nextPipeId == numeric_limits<int>::max())
        {
            report("Закончились свободные ID труб.");
            return true;
        }
        report("Свободной трубы такого диаметра нет. Создайте трубу.");
        pipeId = nextPipeId;
        auto it = pipes.try_emplace(pipeId).first;
        // Диаметр уже выбран при соединении, второй раз его не запрашиваем.
        if (!it->second.input(diameter))
        {
            pipes.erase(it);
            return false;
        }
        ++nextPipeId;
        report("Создана труба, ID: " + to_string(pipeId));
    }
    if (!network.connect(pipeId, inletId, outletId))
    {
        report("Не удалось создать соединение.");
        return true;
    }
    report("Соединение создано: КС " + to_string(inletId) + " -> КС " + to_string(outletId)
        + ", труба ID=" + to_string(pipeId) + ".");
    return true;
}

bool Manager::disconnectPipe()
{
    if (network.getConnections().empty())
    {
        report("В сети нет соединений.");
        return true;
    }
    int pipeId = 0;
    if (!chooseId(pipes, pipeId, "ID трубы для отсоединения (0 - отмена): ")) return false;
    if (pipeId == 0) return true;
    if (!network.isPipeConnected(pipeId)) report("Эта труба уже свободна.");
    else
    {
        network.disconnectPipe(pipeId);
        report("Труба отсоединена, ID: " + to_string(pipeId));
    }
    return true;
}

void Manager::showNetwork() const
{
    if (network.getConnections().empty())
    {
        report("В сети нет соединений.");
        return;
    }
    cout << "Соединения сети (КС входа -> КС выхода):\n";
    for (const auto& [pipeId, connection] : network.getConnections())
        cout << "Труба ID=" << pipeId << ", " << pipes.at(pipeId).getDiameter()
            << " мм: КС " << connection.inletId << " (" << stations.at(connection.inletId).getName()
            << ") -> КС " << connection.outletId << " (" << stations.at(connection.outletId).getName()
            << ")\n";
}

void Manager::sortNetwork() const
{
    if (stations.empty())
    {
        report("КС не добавлены.");
        return;
    }
    vector<int> stationIds;
    for (const auto& [id, station] : stations) stationIds.push_back(id);
    sort(stationIds.begin(), stationIds.end());
    vector<int> order;
    if (!network.topologicalSort(stationIds, order))
    {
        report("Топологическая сортировка невозможна: в сети есть цикл.");
        return;
    }
    cout << "Топологический порядок КС:";
    for (int id : order) cout << ' ' << id;
    cout << '\n';
}
