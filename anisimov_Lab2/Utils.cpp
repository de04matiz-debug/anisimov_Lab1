#include "Utils.h"

bool validName(const std::string& name)
{
    return name.find_first_not_of(" \t\r\n\v\f") != std::string::npos
        && name.find_first_of("\r\n") == std::string::npos;
}

void report(const std::string& message)
{
    std::cout << message << '\n';
}

bool readLine(const std::string& prompt, std::string& line)
{
    std::cout << prompt;
    if (!std::getline(std::cin, line)) return false;
    std::clog << line;
    if (!std::cin.eof()) std::clog << '\n';
    std::clog.flush();
    return static_cast<bool>(std::clog);
}

bool readName(const std::string& prompt, std::string& name)
{
    while (readLine(prompt, name))
    {
        if (validName(name)) return true;
        report("Название не должно быть пустым.");
    }
    return false;
}
