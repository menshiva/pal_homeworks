#include <cstdio>
#include <vector>
#include <memory>

// The main idea of this algorithm is to:
//      1) Build SCCs using Tarjan and then treat this graph as a bunch of connected SCCs (more formally, form a graph condensation).
//      2) Make each SCC contain number of vertices in it and forward/backward edges to other SCCs.
//      3) Find SCC that contains CP.
//      4) Find longest path(s) starting from this SCC using topological sort on forward edges only.
//      5) Do the same on backward edges.
//      6) Number of areas is the sum of longest path distances from 4) and 5) + 1.
//      7) Number of kits is the sum of vertices of all UNIQUE SCCs from the longest path(s) of both 4) and 5).

struct Vertex {
    std::vector<uint32_t> edges;

    uint32_t discTime = 0, lowTime = 0;
    bool inStack = false;

    uint32_t sccIdx = 0;
};

enum EdgeDirection : uint8_t {
    FORWARD = 0,
    BACKWARD,
    NUM
};

struct SccTmp {
    std::vector<uint32_t> vertices;
};

struct Scc {
    std::vector<uint32_t> edges[EdgeDirection::NUM];
    uint32_t verticesNum = 0;

    // this one is used to filter duplicate edges
    uint32_t currentSscAddedFrom = (uint32_t) -1;

    bool visited = false;
    uint32_t dist = 0;
    std::vector<uint32_t> fromSccs;

    bool visitedForKits = false;
};

// I know that global variables is a bad practice, but this is only a single file hw, so...
// All DFS algorithms used in the hw were rewritten to use stack instead of recursion.
// This global variable is used to store DFS stack across all function to preserve memory allocations.
std::vector<uint32_t> _globalStack;

uint32_t readUnsignedInt() {
    // Wrote it just to speed up input a little bit (not needed at all, just for fun and challenge).
    int c;
    while ((c = getchar_unlocked()) < '0');
    uint32_t n = c - '0';
    while ((c = getchar_unlocked()) >= '0')
        n = n * 10 + (c - '0');
    return n;
}

std::vector<Vertex> input(uint32_t &cpIdx) {
    const uint32_t verticesNum = readUnsignedInt();
    const uint32_t edgesNum = readUnsignedInt();
    cpIdx = readUnsignedInt();

    // (void) scanf("%u %u %u", &verticesNum, &edgesNum, &cpIdx);

    std::vector<Vertex> vertices(verticesNum);
    for (auto &v : vertices)
        v.edges.reserve(20); // reserve on average amount of edges

    // uint32_t fromVertIdx, toVertIdx;
    for (uint32_t i = 0; i < edgesNum; ++i) {
        // (void) scanf("%u %u", &fromVertIdx, &toVertIdx);
        // vertices[fromVertIdx].edges.push_back(toVertIdx);
        vertices[readUnsignedInt()].edges.push_back(readUnsignedInt());
    }

    return vertices;
}

void tarjanWithStackBasedDfs(std::vector<Vertex> &vertices, const uint32_t firstVertIdx, std::vector<SccTmp> &outSccsTmp) {
    // Tarjan algorithm to find all SCCs. It is based on DFS with stack (not recursion).
    // Returns SCCs that contains vertices indices.

    static uint32_t time = 0;
    static std::vector<uint32_t> tarjanStack;

    _globalStack.clear();
    _globalStack.push_back(firstVertIdx);

    while (!_globalStack.empty()) {
        const uint32_t vertIdx = _globalStack.back();
        auto &vert = vertices[vertIdx];

        if (vert.discTime == 0) {
            vert.discTime = vert.lowTime = ++time;
            vert.inStack = true;
            tarjanStack.push_back(vertIdx);
        }

        uint32_t i = 0;
        for (; i < (uint32_t) vert.edges.size(); ++i) {
            const uint32_t nextVertIdx = vert.edges[i];
            auto &nextVert = vertices[nextVertIdx];
            if (nextVert.discTime == 0) {
                nextVert.lowTime = nextVert.discTime = ++time;
                nextVert.inStack = true;
                tarjanStack.push_back(nextVertIdx);
                _globalStack.push_back(nextVertIdx);
                break;
            }
            else if (nextVert.inStack)
                vert.lowTime = std::min(vert.lowTime, nextVert.discTime);
        }
        if (i < (uint32_t) vert.edges.size())
            continue;

        if (_globalStack.size() > 1) {
            auto &prevVert = vertices[_globalStack[_globalStack.size() - 2]];
            prevVert.lowTime = std::min(prevVert.lowTime, vert.lowTime);
        }

        if (vert.lowTime == vert.discTime) {
            const auto sccIdx = (uint32_t) outSccsTmp.size();
            outSccsTmp.emplace_back();
            auto &newScc = outSccsTmp.back();

            uint32_t nextVertIdx;
            do {
                nextVertIdx = tarjanStack.back();
                tarjanStack.pop_back();
                auto &nextVertex = vertices[nextVertIdx];
                nextVertex.inStack = false;
                nextVertex.sccIdx = sccIdx;
                newScc.vertices.push_back(nextVertIdx);
            }
            while (nextVertIdx != vertIdx);
        }

        _globalStack.pop_back();
    }
}

std::unique_ptr<Scc[]> buildSCCs(
        std::vector<Vertex> vertices, const uint32_t cpIdx,
        uint32_t &sccsNum, uint32_t &cpSccIdx
) {
    // This function builds an array of SCCs. Each SCC is represented by number of vertices in it and edges to other SCCs.
    // Each SCC contains forward and backward edges (it will be useful for longest path algorithm).
    // Also duplicate edges are removed (because one SCC can lead to other multiple times, and we don't care about it).

    std::vector<SccTmp> sccsTmp;
    for (uint32_t i = 0; i < (uint32_t) vertices.size(); ++i)
        if (vertices[i].discTime == 0)
            tarjanWithStackBasedDfs(vertices, i, sccsTmp);

    // After Tarjan we have SCCs with vertices indices + each Vertex contains index of SCC it belongs to (in Vertex::sccIdx).
    // So now based on this we can build SCCs with number of vertices and edges to other SCCs (that is the only data we need).

    sccsNum = (uint32_t) sccsTmp.size();
    std::unique_ptr<Scc[]> sccs(new Scc[sccsNum]);
    for (uint32_t i = 0; i < sccsNum; ++i) {
        auto &scc = sccs[i];
        auto &sccTmp = sccsTmp[i];

        for (const uint32_t vertIdx : sccTmp.vertices) {
            auto &vert = vertices[vertIdx];
            for (const uint32_t nextVertIdx : vert.edges) {
                const auto &nextVert = vertices[nextVertIdx];
                if (nextVert.sccIdx != i) {
                    auto &nextScc = sccs[nextVert.sccIdx];
                    if (nextScc.currentSscAddedFrom != i) {
                        // this edge leads to other SCC and is unique, so we need to add it index of that SCC to forward edges
                        // and add backward edge to this SCC from current SCC
                        scc.edges[FORWARD].push_back(nextVert.sccIdx);
                        nextScc.edges[BACKWARD].push_back(i);
                        nextScc.currentSscAddedFrom = i;
                    }
                }
            }
        }

        scc.verticesNum = (uint32_t) sccTmp.vertices.size();
    }

    // We don't care about index of CP itself, but index of SCC that contains it.
    cpSccIdx = vertices[cpIdx].sccIdx;
    return sccs;
}

uint32_t topSort(Scc *sccs, const uint32_t cpSccIdx, const EdgeDirection dir, std::vector<uint32_t> &outTopSortStack) {
    // Top sort of SCCs. It is based on DFS with stack (not recursion).
    // Returns top sort stack and number of endpoints (SCCs that have no edges out).
    // This function is used twice: for forward and backward edges.

    uint32_t endPointsNum = 0;

    _globalStack.clear();
    _globalStack.push_back(cpSccIdx);

    while (!_globalStack.empty()) {
        const uint32_t idx = _globalStack.back();

        sccs[idx].visited = true;
        const auto &edges = sccs[idx].edges[dir];

        if (!edges.empty()) {
            uint32_t i = 0;
            for (; i < edges.size(); ++i) {
                const auto nextIdx = edges[i];
                if (!sccs[nextIdx].visited) {
                    _globalStack.push_back(nextIdx);
                    break;
                }
            }
            if (i < edges.size())
                continue;
        }
        else
            ++endPointsNum;

        outTopSortStack.push_back(idx);
        _globalStack.pop_back();
    }

    return endPointsNum;
}

void fillDistancesGetEndpoints(
        Scc *sccs, const EdgeDirection dir, std::vector<uint32_t> &topSortStack,
        std::vector<uint32_t> &outFarthestEndpoints
) {
    // Fills vertex distances based on previous top sort. While doing this, endpoints (SCCs that have no edges out) are collected.
    // This function is used twice: for forward and backward edges.

    for (auto it = topSortStack.crbegin(); it != topSortStack.crend(); ++it) {
        const auto &currentScc = sccs[*it];
        const auto &edges = currentScc.edges[dir];

        if (!edges.empty()) {
            const uint32_t nextDist = currentScc.dist + 1;
            for (const auto nextIdx: edges) {
                auto &nextScc = sccs[nextIdx];
                if (nextScc.dist <= nextDist) {
                    if (nextScc.dist < nextDist) {
                        nextScc.fromSccs.clear();
                        nextScc.dist = nextDist;
                    }
                    nextScc.fromSccs.push_back(*it);
                }
            }
        }
        else
            outFarthestEndpoints.push_back(*it);
    }

    // For now, outFarthestEndpoints contains ALL SCCs that have no edges out, but we only need the farthest ones.
    // So I just "swap" the farthest SCCs to the beginning of the vector in O(n).

    uint32_t minIdx = 0;
    for (uint32_t i = 1; i < outFarthestEndpoints.size(); ++i) {
        const auto idx = outFarthestEndpoints[i];
        if (sccs[outFarthestEndpoints[minIdx]].dist < sccs[idx].dist)
            std::swap(outFarthestEndpoints[minIdx = 0], outFarthestEndpoints[i]);
        else if (sccs[outFarthestEndpoints[minIdx]].dist == sccs[idx].dist)
            std::swap(outFarthestEndpoints[++minIdx], outFarthestEndpoints[i]);
    }
}

uint32_t computeKits(Scc *sccs, std::vector<uint32_t> &paths) {
    // Remove all endpoints that are not farthest.
    const uint32_t maxDist = sccs[paths[0]].dist;
    uint32_t i = 1;
    for (; i < paths.size(); ++i)
        if (sccs[paths[i]].dist != maxDist)
            break;
    paths.erase(paths.begin() + i, paths.end());

    // Now we have only farthest endpoints, so we can compute number of kits.
    // We need to visit each SCC only once, so we use visitedForKits flag.

    uint32_t kits = 0;

    for (const auto sccIdx : paths) {
        sccs[sccIdx].visitedForKits = true;
        kits += sccs[sccIdx].verticesNum;
    }

    // we can't use iterators or for each here because we are adding new elements to the vector simultaneously
    for (uint32_t j = 0; j < paths.size(); ++j) {
        auto &scc = sccs[paths[j]];
        for (const auto fromIdx : scc.fromSccs) {
            auto &fromScc = sccs[fromIdx];
            if (!fromScc.visitedForKits) {
                fromScc.visitedForKits = true;
                kits += fromScc.verticesNum;
                paths.push_back(fromIdx);
            }
        }
    }

    return kits;
}

int main() {
    uint32_t cpIdx;
    auto vertices = input(cpIdx);

    uint32_t sccsNum, cpSccIdx;
    auto sccs = buildSCCs(std::move(vertices), cpIdx, sccsNum, cpSccIdx);

    std::vector<uint32_t> topSortStack;
    std::vector<uint32_t> farthestEndpoints;

    uint32_t areas = 0, kits = 0;

    for (uint8_t dir = FORWARD; dir < NUM; ++dir) {
        topSortStack.clear();
        topSortStack.reserve(sccsNum);
        const auto endPointsNum = topSort(sccs.get(), cpSccIdx, (EdgeDirection) dir, topSortStack);

        farthestEndpoints.clear();
        farthestEndpoints.reserve(endPointsNum);
        fillDistancesGetEndpoints(sccs.get(), (EdgeDirection) dir, topSortStack, farthestEndpoints);

        areas += sccs[farthestEndpoints[0]].dist;
        kits += computeKits(sccs.get(), farthestEndpoints);
    }

    printf("%u %u\n", areas + 1, kits);
    return 0;
}
