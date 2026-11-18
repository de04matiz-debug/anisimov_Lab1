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
        << "6. Сохранить\n7. Загрузить\n8. Поиск объектов и пакетные операции с трубами\n"
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
    std::streambuf* previousLog = std::clog.rdbuf(log.rdbuf());
    Manager manager;
    bool running = true;
    while (running && std::clog)
    {
        showMenu();
        int choice = 0;
        if (!readNumber("Выберите действие: ", choice, 0, 9)) break;
        std::string fileName;
        switch (choice)
        {
        case 0: running = false; break;
        case 1: running = manager.addPipe(); break;
        case 2: running = manager.addStation(); break;
        case 3: manager.showObjects(); break;
        case 4: running = manager.editPipe(); break;
        case 5: running = manager.editStation(); break;
        case 6:
        case 7:
            running = readName("Имя файла или полный путь: ", fileName);
            if (running)
            {
                if (choice == 6) manager.saveToFile(fileName);
                else manager.loadFromFile(fileName);
            }
            break;
        case 8: running = manager.searchObjects(); break;
        case 9: running = manager.deleteObject(); break;
        }
    }
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
