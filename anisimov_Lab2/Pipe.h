#pragma once
#include <iosfwd>
#include <string>

class Pipe
{
    std::string name;
    double length = 0;
    int diameter = 0;
    bool inRepair = false;

public:
    const std::string& getName() const { return name; }
    bool isRepairing() const { return inRepair; }
    void setRepairing(bool value) { inRepair = value; }
    bool input();
    void show(int id) const;
    friend std::ostream& operator<<(std::ostream& out, const Pipe& pipe);
    friend std::istream& operator>>(std::istream& in, Pipe& pipe);
};
