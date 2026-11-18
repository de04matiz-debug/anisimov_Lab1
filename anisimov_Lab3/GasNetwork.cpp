#include "GasNetwork.h"
#include <queue>
#include <unordered_map>

bool GasNetwork::isPipeConnected(int pipeId) const
{
    return connections.count(pipeId) != 0;
}

bool GasNetwork::connect(int pipeId, int inletId, int outletId)
{
    if (pipeId <= 0 || inletId <= 0 || outletId <= 0 || inletId == outletId) return false;
    return connections.emplace(pipeId, Connection{ inletId, outletId }).second;
}

void GasNetwork::disconnectPipe(int pipeId)
{
    connections.erase(pipeId);
}

void GasNetwork::disconnectStation(int stationId)
{
    for (auto it = connections.begin(); it != connections.end(); )
    {
        if (it->second.inletId == stationId || it->second.outletId == stationId)
            it = connections.erase(it);
        else ++it;
    }
}

bool GasNetwork::topologicalSort(const std::vector<int>& stationIds, std::vector<int>& order) const
{
    order.clear();
    std::unordered_map<int, int> indegree;
    std::unordered_map<int, std::vector<int>> adjacent;
    for (int id : stationIds) indegree.emplace(id, 0);
    for (const auto& [pipeId, connection] : connections)
    {
        if (!indegree.count(connection.inletId) || !indegree.count(connection.outletId)) return false;
        adjacent[connection.inletId].push_back(connection.outletId);
        ++indegree.at(connection.outletId);
    }

    // Алгоритм Кана: сначала берём КС без входящих рёбер.
    std::queue<int> ready;
    for (int id : stationIds)
        if (indegree.at(id) == 0) ready.push(id);
    while (!ready.empty())
    {
        const int id = ready.front();
        ready.pop();
        order.push_back(id);
        for (int next : adjacent[id])
            if (--indegree.at(next) == 0) ready.push(next);
    }
    if (order.size() == stationIds.size()) return true;
    // При цикле частичный порядок не является топологической сортировкой.
    order.clear();
    return false;
}
