#include "Utils.h"
#include <ctime>
#include <iomanip>

bool validName(const std::string& name)
{
    return name.find_first_not_of(" \t\r\n") != std::string::npos
        && name.find_first_of("\r\n") == std::string::npos;
}

void logAction(const std::string& message)
{
    const std::time_t now = std::time(nullptr);
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &now);
#else
    local = *std::localtime(&now);
#endif
    std::clog << std::put_time(&local, "%Y-%m-%d %H:%M:%S") << ' ' << message << std::endl;
}

bool readLine(const std::string& prompt, std::string& line)
{
    std::cout << prompt;
    if (!std::getline(std::cin, line))
    {
        logAction("Конец ввода: " + prompt);
        return false;
    }
    std::ostringstream entry;
    entry << "Ввод [" << prompt << "] " << std::quoted(line);
    logAction(entry.str());
    return true;
}

bool readName(const std::string& prompt, std::string& name)
{
    while (readLine(prompt, name))
    {
        if (validName(name)) return true;
        logAction("Ошибка: пустое название");
        std::cout << "Название не должно быть пустым.\n";
    }
    return false;
}
