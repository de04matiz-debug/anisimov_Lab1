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
    int getDiameter() const { return diameter; }
    bool isRepairing() const { return inRepair; }
    void setRepairing(bool value) { inRepair = value; }
    bool input(int selectedDiameter = 0);
    void show(int id) const;
    friend std::ostream& operator<<(std::ostream& out, const Pipe& pipe);
    friend std::istream& operator>>(std::istream& in, Pipe& pipe);
};
