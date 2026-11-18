#pragma once
#include <map>
#include <vector>

struct Connection
{
    int inletId;
    int outletId;
};

class GasNetwork
{
    // Ключ — ID трубы: одна труба может соединять только одну пару КС.
    std::map<int, Connection> connections;

public:
    bool isPipeConnected(int pipeId) const;
    bool connect(int pipeId, int inletId, int outletId);
    void disconnectPipe(int pipeId);
    void disconnectStation(int stationId);
    void clear() { connections.clear(); }
    const std::map<int, Connection>& getConnections() const { return connections; }
    bool topologicalSort(const std::vector<int>& stationIds, std::vector<int>& order) const;
};
