#include "Manager.h"
#include "Utils.h"
#include <fstream>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

void showMenu()
{
    std::cout << "\n1. Добавить трубу\n2. Добавить КС\n3. Просмотр объектов\n"
        << "4. Изменить ремонт трубы по ID\n5. Запуск или остановка цеха КС по ID\n"
        << "6. Сохранить\n7. Загрузить\n8. Поиск и пакетные операции с трубами\n"
        << "9. Удалить объект по ID\n0. Выход\n";
}

int main()
{
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif
    std::ofstream log("log.txt", std::ios::app);
    if (!log)
    {
        std::cerr << "Не удалось открыть log.txt. Работа без журнала невозможна.\n";
        return 1;
    }
    // Все сообщения std::clog отправляются в файл, std::cin и std::cout не перенаправляются.
    std::streambuf* previousLog = std::clog.rdbuf(log.rdbuf());
    logAction("Начало работы ЛР2");
    AppState state;
    bool running = true;
    while (running && std::clog)
    {
        showMenu();
        int choice = 0;
        if (!readNumber("Выберите действие: ", choice, 0, 9)) break;
        logAction("Выбрано действие: " + std::to_string(choice));
        std::string fileName;
        switch (choice)
        {
        case 0: running = false; break;
        case 1: running = addPipe(state); break;
        case 2: running = addStation(state); break;
        case 3: showObjects(state); break;
        case 4: running = editPipe(state); break;
        case 5: running = editStation(state); break;
        case 6:
        case 7:
            running = readName("Путь к существующему файлу: ", fileName);
            if (running)
            {
                if (choice == 6) saveToFile(fileName, state);
                else loadFromFile(fileName, state);
            }
            break;
        case 8: running = searchObjects(state); break;
        case 9: running = deleteObject(state); break;
        }
    }
    logAction("Завершение работы ЛР2");
    const bool logOk = static_cast<bool>(std::clog);
    std::clog.rdbuf(previousLog);
    log.close();
    if (!logOk || !log)
    {
        std::cerr << "Ошибка записи журнала.\n";
        return 1;
    }
    std::cout << "Программа завершена.\n";
    return 0;
}
