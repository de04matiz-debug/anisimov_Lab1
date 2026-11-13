#include "Utils.h"
#include <ctime>

bool validName(const std::string& name)
{
    return name.find_first_not_of(" \t\r\n\v\f") != std::string::npos
        && name.find_first_of("\r\n") == std::string::npos;
}

void logAction(const std::string& message)
{
    const auto now = std::time(nullptr);
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &now);
#else
    local = *std::localtime(&now);
#endif
    char stamp[20]{};
    std::strftime(stamp, sizeof(stamp), "%Y-%m-%d %H:%M:%S", &local);
    std::clog << stamp << ' ' << message << std::endl;
}

void report(const std::string& message)
{
    std::cout << message << '\n';
    logAction(message);
}

bool readLine(const std::string& prompt, std::string& line)
{
    std::cout << prompt;
    if (!std::getline(std::cin, line))
    {
        logAction("Конец ввода: " + prompt);
        return false;
    }
    logAction("Ввод [" + prompt + "] " + line);
    return true;
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
