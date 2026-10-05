#define main labProgramMain
#include "../anisimov_Lab1/anisimov_Lab1.cpp"
#undef main
#include <cassert>
#include <vector>

void writeFixture(const string& path, const string& content)
{
    ofstream file(path);
    file << content;
    file.close();
    assert(file);
}

bool sameState(const AppState& left, const AppState& right)
{
    return left.hasPipe == right.hasPipe && left.hasStation == right.hasStation
        && left.pipe.name == right.pipe.name && left.pipe.length == right.pipe.length
        && left.pipe.diameter == right.pipe.diameter && left.pipe.inRepair == right.pipe.inRepair
        && left.station.name == right.station.name
        && left.station.workshopCount == right.station.workshopCount
        && left.station.workingWorkshops == right.station.workingWorkshops
        && left.station.stationClass == right.station.stationClass;
}

int main(int argc, char* argv[])
{
    if (argc != 2) return 1; // A separate temporary directory, never the user's files.
    const string directory = argv[1];
    filesystem::create_directories(directory);
    const string path = directory + "/state.txt";
    AppState original;
    original.pipe = { "Pipe km 125", 12.123456789012345, 700, true };
    original.station = { "Station North", 4, 2, 1 };
    original.hasPipe = original.hasStation = true;
    int checks = 0;

    for (int mask = 0; mask < 4; ++mask)
    {
        AppState expected = original;
        expected.hasPipe = (mask & 1) != 0;
        expected.hasStation = (mask & 2) != 0;
        writeFixture(path, "old data");
        assert(saveToFile(path, expected));
        AppState actual = original;
        assert(loadFromFile(path, actual));
        if (!expected.hasPipe) expected.pipe = {};
        if (!expected.hasStation) expected.station = {};
        assert(sameState(actual, expected));
        ++checks;
    }

    const vector<string> invalidFiles = {
        "", "0\n", "-1\n0\n", "2\n0\n", "x\n0\n", "0\n2\n", "0\n-1\n",
        "1\nPipe\n12.5\n700\n", "1\n\n12.5\n700\n0\n0\n",
        "1\n   \n12.5\n700\n0\n0\n", "1\nPipe\n0\n700\n0\n0\n",
        "1\nPipe\n-1\n700\n0\n0\n", "1\nPipe\nnan\n700\n0\n0\n",
        "1\nPipe\ninf\n700\n0\n0\n", "1\nPipe\n1e999\n700\n0\n0\n",
        "1\nPipe\n12.5x\n700\n0\n0\n", "1\nPipe\n12.5\n0\n0\n0\n",
        "1\nPipe\n12.5\n700.5\n0\n0\n", "1\nPipe\n12.5\n700\n2\n0\n",
        "0\n1\nStation\n4\n5\n1\n", "0\n1\nStation\n4\n-1\n1\n",
        "0\n1\nStation\n0\n0\n1\n", "0\n1\nStation\n4\n2\n0\n",
        "0\n1\nStation\n4\n2\n", "0\n0\nextra\n", "0\n0\n\n",
        "0\n0 1\n", "0\n1\nStation\n9999999999999999\n0\n1\n"
    };
    for (const string& content : invalidFiles)
    {
        writeFixture(path, content);
        AppState actual = original;
        assert(!loadFromFile(path, actual));
        assert(sameState(actual, original));
        ++checks;
    }
    writeFixture(path, "0\n0");
    AppState empty = original;
    assert(loadFromFile(path, empty) && !empty.hasPipe && !empty.hasStation);
    ++checks;

    const string missing = directory + "/never-created.txt";
    assert(!filesystem::exists(missing));
    assert(!saveToFile(missing, original));
    assert(!filesystem::exists(missing));
    AppState actual = original;
    assert(!loadFromFile(missing, actual) && sameState(actual, original));
    ++checks;
    assert(!saveToFile(directory, original));
    assert(!loadFromFile(directory, actual) && sameState(actual, original));
    ++checks;

    writeFixture(path, "0\n0\n");
    AppState badState = original;
    badState.pipe.length = -1;
    assert(!saveToFile(path, badState));
    ifstream unchanged(path);
    string content((istreambuf_iterator<char>(unchanged)), istreambuf_iterator<char>());
    assert(content == "0\n0\n");
    ++checks;

    for (const string& value : { "2.5", "3x", "", "3 4", "9999999999999999" })
    {
        istringstream input(value);
        int number = 10;
        assert(!readNumberLine(input, number) && number == 10);
        ++checks;
    }
    cout << "PASS: " << checks << " persistence/input checks.\n";
    return 0;
}
