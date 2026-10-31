#pragma once
#include <iosfwd>
#include <string>

class Pipe
{
    int id = 0;
    std::string name;
    double length = 0;
    int diameter = 0;
    bool inRepair = false;

public:
    explicit Pipe(int objectId = 0) : id(objectId) {}
    int getId() const { return id; }
    const std::string& getName() const { return name; }
    bool isRepairing() const { return inRepair; }
    void setRepairing(bool value) { inRepair = value; }
    bool valid() const;
    bool input();
    void show() const;
    friend std::ostream& operator<<(std::ostream& out, const Pipe& pipe);
    friend std::istream& operator>>(std::istream& in, Pipe& pipe);
};
