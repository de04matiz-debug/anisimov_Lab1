#include <iostream>
#include <string>
#include <sstream>
#include <limits>
#include <cmath>

using namespace std;

struct Pipe
{
    string name;
    double length = 0;
    int diameter = 0;
    bool inRepair = false;
};

struct CompressorStation
{
    string name;
    int workshopCount = 0;
    int workingWorkshops = 0;
    int stationClass = 0;
};

struct AppState
{
    Pipe pipe;
    CompressorStation station;
    bool hasPipe = false;
    bool hasStation = false;
};

bool validName(const string& name)
{
    return name.find_first_not_of(" \t\r\n") != string::npos
        && name.find_first_of("\r\n") == string::npos;
}

// Each number occupies one line. Reject trailing text and overflow.
template <typename T>
bool readNumberLine(istream& in, T& value)
{
    string line;
    if (!getline(in, line))
        return false;
    istringstream row(line);
    T candidate{};
    if (!(row >> candidate) || !isfinite(static_cast<double>(candidate)))
        return false;
    row >> ws;
    if (!row.eof())
        return false;
    value = candidate;
    return true;
}

template <typename T>
bool readNumber(const string& prompt, T& value, T minimum, T maximum)
{
    while (true)
    {
        cout << prompt;
        if (readNumberLine(cin, value) && value >= minimum && value <= maximum)
            return true;
        if (!cin)
            return false;
        cout << "Invalid input. Allowed range: " << minimum << " .. " << maximum << ".\n";
    }
}

bool readName(const string& prompt, string& name)
{
    while (true)
    {
        cout << prompt;
        if (!getline(cin, name))
            return false;
        if (validName(name))
            return true;
        cout << "The name must not be empty.\n";
    }
}

bool addPipe(Pipe& pipe)
{
    Pipe candidate;
    int repair;
    if (!readName("Pipe name (kilometer mark): ", candidate.name)
        || !readNumber("Length (km, > 0): ", candidate.length,
            numeric_limits<double>::denorm_min(), numeric_limits<double>::max())
        || !readNumber("Diameter (mm, > 0): ", candidate.diameter, 1, numeric_limits<int>::max())
        || !readNumber("Under repair (0 - no, 1 - yes): ", repair, 0, 1))
        return false;
    candidate.inRepair = (repair == 1);
    pipe = candidate;
    cout << "Pipe added (replaces the previous pipe).\n";
    return true;
}

bool addStation(CompressorStation& station)
{
    CompressorStation candidate;
    if (!readName("Station name: ", candidate.name)
        || !readNumber("Number of workshops (> 0): ", candidate.workshopCount, 1, numeric_limits<int>::max())
        || !readNumber("Working workshops: ", candidate.workingWorkshops, 0, candidate.workshopCount)
        || !readNumber("Station class (> 0): ", candidate.stationClass, 1, numeric_limits<int>::max()))
        return false;
    station = candidate;
    cout << "Station added (replaces the previous station).\n";
    return true;
}

void showPipe(const Pipe& pipe)
{
    cout << "\nPIPE\nName: " << pipe.name
        << "\nLength: " << pipe.length << " km\nDiameter: " << pipe.diameter
        << " mm\nUnder repair: " << (pipe.inRepair ? "Yes" : "No") << '\n';
}

void showStation(const CompressorStation& station)
{
    cout << "\nCOMPRESSOR STATION\nName: " << station.name
        << "\nNumber of workshops: " << station.workshopCount
        << "\nWorking workshops: " << station.workingWorkshops
        << "\nStation class: " << station.stationClass << '\n';
}

bool editPipe(Pipe& pipe)
{
    int repair;
    if (!readNumber("Under repair (0 - no, 1 - yes): ", repair, 0, 1))
        return false;
    pipe.inRepair = (repair == 1);
    cout << "Pipe state changed.\n";
    return true;
}

bool editStation(CompressorStation& station)
{
    int action;
    if (!readNumber("1 - Start a workshop, 2 - Stop a workshop, 0 - Cancel: ", action, 0, 2))
        return false;
    if (action == 1)
    {
        if (station.workingWorkshops == station.workshopCount)
            cout << "All workshops are already working.\n";
        else
        {
            ++station.workingWorkshops;
            cout << "Workshop started.\n";
        }
    }
    else if (action == 2)
    {
        if (station.workingWorkshops == 0)
            cout << "There are no working workshops.\n";
        else
        {
            --station.workingWorkshops;
            cout << "Workshop stopped.\n";
        }
    }
    return true;
}

void showMenu()
{
    cout << "\n1. Add pipe\n2. Add compressor station\n3. Show all objects\n"
        << "4. Edit pipe\n5. Edit compressor station\n6. Save\n7. Load\n0. Exit\n";
}

int main()
{
    AppState state;
    int choice;
    while (true)
    {
        showMenu();
        if (!readNumber("Choose an option: ", choice, 0, 7) || choice == 0)
            break;
        switch (choice)
        {
        case 1:
            if (!addPipe(state.pipe)) return 0;
            state.hasPipe = true;
            break;
        case 2:
            if (!addStation(state.station)) return 0;
            state.hasStation = true;
            break;
        case 3:
            if (state.hasPipe) showPipe(state.pipe);
            else cout << "Pipe is not created.\n";
            if (state.hasStation) showStation(state.station);
            else cout << "Station is not created.\n";
            break;
        case 4:
            if (!state.hasPipe) cout << "First add a pipe.\n";
            else if (!editPipe(state.pipe)) return 0;
            break;
        case 5:
            if (!state.hasStation) cout << "First add a station.\n";
            else if (!editStation(state.station)) return 0;
            break;
        case 6:
        case 7:
            cout << "File operations are being updated.\n";
            break;
        }
    }
    cout << "Program finished.\n";
    return 0;
}
