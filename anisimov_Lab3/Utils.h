#pragma once
#include <charconv>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

bool validName(const std::string& name);
void report(const std::string& message);
bool readLine(const std::string& prompt, std::string& line);
bool readName(const std::string& prompt, std::string& name);
bool validDiameter(int diameter);
bool readDiameter(const std::string& prompt, int& diameter);

template <typename T>
bool parseNumber(const std::string& line, T& value)
{
    auto first = line.find_first_not_of(" \t\r\n\v\f");
    if (first == std::string::npos) return false;
    const auto last = line.find_last_not_of(" \t\r\n\v\f") + 1;
    if (line[first] == '+')
    {
        ++first;
        if (first == last || line[first] == '-' || line[first] == '+') return false;
    }
    const auto result = std::from_chars(line.data() + first, line.data() + last, value);
    return result.ec == std::errc{} && result.ptr == line.data() + last
        && std::isfinite(static_cast<double>(value));
}

template <typename T>
bool readNumberLine(std::istream& in, T& value)
{
    std::string line;
    return std::getline(in, line) && parseNumber(line, value);
}

template <typename T>
bool readNumber(const std::string& prompt, T& value, T minimum, T maximum)
{
    std::string line;
    while (readLine(prompt, line))
    {
        if (parseNumber(line, value) && value >= minimum && value <= maximum) return true;
        report("Ошибка ввода. Повторите ввод.");
    }
    return false;
}

template <typename Objects, typename Filter>
std::vector<int> findIds(const Objects& objects, Filter filter)
{
    std::vector<int> ids;
    for (const auto& [id, object] : objects)
        if (filter(object)) ids.push_back(id);
    return ids;
}
