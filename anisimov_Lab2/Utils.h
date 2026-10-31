#pragma once

#include <cmath>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <vector>

bool validName(const std::string& name);
void logAction(const std::string& message);
bool readLine(const std::string& prompt, std::string& line);
bool readName(const std::string& prompt, std::string& name);

// Число должно занимать всю строку, как в ЛР1.
template <typename T>
bool readNumberLine(std::istream& in, T& value)
{
    std::string line;
    if (!std::getline(in, line)) return false;
    std::istringstream row(line);
    T candidate{};
    if (!(row >> candidate) || !std::isfinite(candidate)) return false;
    row >> std::ws;
    if (!row.eof()) return false;
    value = candidate;
    return true;
}

template <typename T>
bool readNumber(const std::string& prompt, T& value, T minimum, T maximum)
{
    while (true)
    {
        std::string line;
        if (!readLine(prompt, line)) return false;
        std::istringstream row(line);
        T candidate{};
        if (readNumberLine(row, candidate) && candidate >= minimum && candidate <= maximum)
        {
            value = candidate;
            return true;
        }
        logAction("Ошибка ввода числа: " + prompt);
        std::cout << "Ошибка ввода. Повторите ввод.\n";
    }
}

// Один алгоритм поиска для обеих коллекций, условие передаётся отдельно.
template <typename T, typename Filter>
std::vector<int> findIds(const std::map<int, T>& objects, Filter filter)
{
    std::vector<int> ids;
    for (const auto& [id, object] : objects)
        if (filter(object)) ids.push_back(id);
    return ids;
}
