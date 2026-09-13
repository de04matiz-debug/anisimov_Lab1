#include <iostream>
#include <string>
#include <fstream>

using namespace std;

struct Pipe
{
    string name;
    double length;
    int diameter;
    bool inRepair;
    bool created;
};

struct CompressorStation
{
    string name;
    int workshopCount;
    int workingWorkshops;
    int stationClass;
    bool created;
};

void addPipe(Pipe& pipe)
{
    cout << "\n--- Add pipe ---\n";

    cout << "Enter pipe name: ";
    cin >> pipe.name;

    cout << "Enter pipe length (km): ";
    while (!(cin >> pipe.length) || pipe.length < 0)
    {
        cout << "Error. Enter a positive number: ";
        cin.clear();
        cin.ignore(1000, '\n');
    }

    cout << "Enter pipe diameter (mm): ";
    while (!(cin >> pipe.diameter) || pipe.diameter <= 0)
    {
        cout << "Error. Enter a positive number: ";
        cin.clear();
        cin.ignore(1000, '\n');
    }

    int answer;

    cout << "Is the pipe under repair?\n";
    cout << "1 - Yes\n";
    cout << "0 - No\n";

    while (!(cin >> answer) || (answer != 0 && answer != 1))
    {
        cout << "Error. Enter 1 or 0: ";
        cin.clear();
        cin.ignore(1000, '\n');
    }

    pipe.inRepair = (answer == 1);
    pipe.created = true;

    cout << "Pipe added.\n";
}

void addStation(CompressorStation& station)
{
    cout << "\n--- Add compressor station ---\n";

    cout << "Enter station name: ";
    cin >> station.name;

    cout << "Enter number of workshops: ";
    while (!(cin >> station.workshopCount) || station.workshopCount <= 0)
    {
        cout << "Error. Enter a positive number: ";
        cin.clear();
        cin.ignore(1000, '\n');
    }

    cout << "Enter number of working workshops: ";
    while (!(cin >> station.workingWorkshops) ||
        station.workingWorkshops < 0 ||
        station.workingWorkshops > station.workshopCount)
    {
        cout << "Error. Number of working workshops must be from 0 to "
            << station.workshopCount << ": ";
        cin.clear();
        cin.ignore(1000, '\n');
    }

    cout << "Enter station class: ";
    while (!(cin >> station.stationClass) || station.stationClass <= 0)
    {
        cout << "Error. Enter a positive number: ";
        cin.clear();
        cin.ignore(1000, '\n');
    }

    station.created = true;

    cout << "Compressor station added.\n";
}

void showObjects(Pipe pipe, CompressorStation station)
{
    cout << "\n===== ALL OBJECTS =====\n";

    if (pipe.created)
    {
        cout << "\nPIPE\n";
        cout << "Name: " << pipe.name << "\n";
        cout << "Length: " << pipe.length << " km\n";
        cout << "Diameter: " << pipe.diameter << " mm\n";
        cout << "Under repair: ";

        if (pipe.inRepair)
            cout << "Yes\n";
        else
            cout << "No\n";
    }
    else
    {
        cout << "\nPipe is not created.\n";
    }

    if (station.created)
    {
        cout << "\nCOMPRESSOR STATION\n";
        cout << "Name: " << station.name << "\n";
        cout << "Number of workshops: "
            << station.workshopCount << "\n";
        cout << "Working workshops: "
            << station.workingWorkshops << "\n";
        cout << "Station class: "
            << station.stationClass << "\n";
    }
    else
    {
        cout << "\nCompressor station is not created.\n";
    }
}

void editPipe(Pipe& pipe)
{
    if (!pipe.created)
    {
        cout << "First add a pipe.\n";
        return;
    }

    int answer;

    cout << "\n--- Edit pipe ---\n";
    cout << "1 - Under repair\n";
    cout << "0 - Not under repair\n";

    while (!(cin >> answer) || (answer != 0 && answer != 1))
    {
        cout << "Error. Enter 1 or 0: ";
        cin.clear();
        cin.ignore(1000, '\n');
    }

    pipe.inRepair = (answer == 1);

    cout << "Pipe state changed.\n";
}

void editStation(CompressorStation& station)
{
    if (!station.created)
    {
        cout << "First add a compressor station.\n";
        return;
    }

    int answer;

    cout << "\n--- Edit compressor station ---\n";
    cout << "1 - Start a workshop\n";
    cout << "2 - Stop a workshop\n";
    cout << "0 - Cancel\n";

    while (!(cin >> answer) || answer < 0 || answer > 2)
    {
        cout << "Error. Enter 0, 1 or 2: ";
        cin.clear();
        cin.ignore(1000, '\n');
    }

    if (answer == 1)
    {
        if (station.workingWorkshops < station.workshopCount)
        {
            station.workingWorkshops++;
            cout << "Workshop started.\n";
        }
        else
        {
            cout << "All workshops are already working.\n";
        }
    }

    if (answer == 2)
    {
        if (station.workingWorkshops > 0)
        {
            station.workingWorkshops--;
            cout << "Workshop stopped.\n";
        }
        else
        {
            cout << "There are no working workshops.\n";
        }
    }
}

void saveData(Pipe pipe, CompressorStation station)
{
    ofstream file("data.txt");

    if (!file)
    {
        cout << "Error opening file.\n";
        return;
    }

    file << pipe.created << "\n";

    if (pipe.created)
    {
        file << pipe.name << "\n";
        file << pipe.length << "\n";
        file << pipe.diameter << "\n";
        file << pipe.inRepair << "\n";
    }

    file << station.created << "\n";

    if (station.created)
    {
        file << station.name << "\n";
        file << station.workshopCount << "\n";
        file << station.workingWorkshops << "\n";
        file << station.stationClass << "\n";
    }

    file.close();

    cout << "Data saved.\n";
}

void loadData(Pipe& pipe, CompressorStation& station)
{
    ifstream file("data.txt");

    if (!file)
    {
        cout << "File not found.\n";
        return;
    }

    file >> pipe.created;
    file.ignore(1000, '\n');

    if (pipe.created)
    {
        getline(file, pipe.name);
        file >> pipe.length;
        file >> pipe.diameter;
        file >> pipe.inRepair;
        file.ignore(1000, '\n');
    }

    file >> station.created;
    file.ignore(1000, '\n');

    if (station.created)
    {
        getline(file, station.name);
        file >> station.workshopCount;
        file >> station.workingWorkshops;
        file >> station.stationClass;
    }

    file.close();

    cout << "Data loaded.\n";
}

int main()
{
    Pipe pipe = {};
    CompressorStation station = {};

    int choice;

    while (true)
    {
        cout << "\n========================\n";
        cout << "1. Add pipe\n";
        cout << "2. Add compressor station\n";
        cout << "3. Show all objects\n";
        cout << "4. Edit pipe\n";
        cout << "5. Edit compressor station\n";
        cout << "6. Save\n";
        cout << "7. Load\n";
        cout << "0. Exit\n";
        cout << "========================\n";

        cout << "Choose an option: ";

        if (!(cin >> choice))
        {
            cout << "Error. Enter a number from 0 to 7.\n";
            cin.clear();
            cin.ignore(1000, '\n');
            continue;
        }

        if (choice == 1)
            addPipe(pipe);

        else if (choice == 2)
            addStation(station);

        else if (choice == 3)
            showObjects(pipe, station);

        else if (choice == 4)
            editPipe(pipe);

        else if (choice == 5)
            editStation(station);

        else if (choice == 6)
            saveData(pipe, station);

        else if (choice == 7)
            loadData(pipe, station);

        else if (choice == 0)
        {
            cout << "Program finished.\n";
            break;
        }

        else
        {
            cout << "There is no such menu option.\n";
        }
    }

    return 0;
}