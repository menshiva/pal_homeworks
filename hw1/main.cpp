#include <cstdio>
#include <cstdint>
#include <vector>
#include <queue>
#include <algorithm>
#include <limits>

struct KruskalEdge {
    bool operator<(const KruskalEdge rhs) const {
        return weight < rhs.weight;
    }

    uint16_t from, to;
    int32_t weight;
};

struct KruskalVertex {
    void reset() {
        boss = -1;
        rank = 0;
    }

    int16_t boss = -1;
    uint8_t rank = 0;
};

struct ClassKruskalVertex {
    void reset() {
        connectedToPrevious = false;
        boss = -1;
        rank = 0;
    }

    bool connectedToPrevious = false;
    int16_t boss = -1;
    uint8_t rank = 0;
};

struct Edge {
    Edge(const uint16_t to, const int32_t weight) : to(to), weight(weight) {}
    uint16_t to;
    int32_t weight;
};

// I know that global variables is a bad practice, but this is only a single file hw, so
KruskalVertex _kruskalVerticesInsideClass[500];
uint16_t _kruskalVerticesInsideClassNum;
KruskalEdge _kruskalEdgesInsideClass[10000];
uint16_t _kruskalEdgesInsideClassNum;

struct Vertex {
    uint16_t getOrCreateKruskalVertexInsideClassIdx() {
        if (kruskalVertexIdx < 0)
            _kruskalVerticesInsideClass[kruskalVertexIdx = (int16_t) (_kruskalVerticesInsideClassNum++)].reset();
        return kruskalVertexIdx;
    }

    void reset() {
        // clear vertex variables needed for computations
        dist = -1;
        classIdx = -1;
        kruskalVertexIdx = -1;
    }

    std::vector<Edge> edges;
    int16_t dist = -1, classIdx = -1;
    int16_t kruskalVertexIdx = -1;
};

using Class = std::vector<uint16_t>;

// I know that global variables is a bad practice, but this is only a single file hw, so
Vertex _vertices[500];
uint16_t _verticesNum;

ClassKruskalVertex _kruskalClasses[500];
uint16_t _kruskalClassesNum;
KruskalEdge _kruskalEdgesBetweenClasses[10000];
uint16_t _kruskalEdgesBetweenClassesNum;

template <typename Vert>
uint16_t ufFindParent(Vert *kruskalVertices, const uint16_t idx) {
    return kruskalVertices[idx].boss >= 0 ? (kruskalVertices[idx].boss = ufFindParent(kruskalVertices, kruskalVertices[idx].boss)) : idx;
}

template <typename Vert>
void ufUnion(Vert *kruskalVertices, const uint16_t idxA, const uint16_t idxB) {
    auto &vA = kruskalVertices[idxA];
    auto &vB = kruskalVertices[idxB];
    if (vA.rank < vB.rank)
        vA.boss = idxB;
    else {
        vB.boss = idxA;
        if (vA.rank == vB.rank)
            ++vA.rank;
    }
}

void kruskal(
        KruskalVertex *kruskalVertices, uint16_t kruskalVerticesNum,
        KruskalEdge *kruskalEdges, const uint16_t kruskalEdgesNum,
        long long &res
) {
    --kruskalVerticesNum;
    uint16_t addedEdgesNum = 0;

    std::sort(kruskalEdges, kruskalEdges + kruskalEdgesNum);

    for (uint16_t i = 0; i < kruskalEdgesNum; ++i) {
        const auto e = kruskalEdges[i];
        const uint16_t bossFrom = ufFindParent(kruskalVertices, e.from);
        const uint16_t bossTo = ufFindParent(kruskalVertices, e.to);
        if (bossFrom != bossTo) {
            ufUnion(kruskalVertices, bossFrom, bossTo);
            res += (long long) e.weight;
            if (++addedEdgesNum == kruskalVerticesNum)
                return;
        }
    }
}

bool kruskal(
        ClassKruskalVertex *kruskalVertices, uint16_t kruskalVerticesNum,
        KruskalEdge *kruskalEdges, const uint16_t kruskalEdgesNum,
        long long &res
) {
    --kruskalVerticesNum;
    uint16_t addedEdgesNum = 0;

    std::sort(kruskalEdges, kruskalEdges + kruskalEdgesNum);

    for (uint16_t i = 0; i < kruskalEdgesNum; ++i) {
        const auto e = kruskalEdges[i];

        auto &toV = kruskalVertices[e.to];
        if (toV.connectedToPrevious)
            continue;

        const uint16_t bossFrom = ufFindParent(kruskalVertices, e.from);
        const uint16_t bossTo = ufFindParent(kruskalVertices, e.to);

        if (bossFrom != bossTo) {
            toV.connectedToPrevious = true;
            ufUnion(kruskalVertices, bossFrom, bossTo);
            res += (long long) e.weight;
            if (++addedEdgesNum == kruskalVerticesNum)
                return true;
        }
    }

    return false;
}

Class fillClassKruskalDataAndBuildClass(const uint16_t startVertIdx, const int16_t neededDist) {
    Class result;

    const uint16_t newClassIdx = _kruskalClassesNum;
    _kruskalClasses[_kruskalClassesNum++].reset();

    std::queue<uint16_t> bfsQueue;
    bfsQueue.push(startVertIdx);

    while (!bfsQueue.empty()) {
        const uint16_t fromVidx = bfsQueue.front();
        bfsQueue.pop();

        auto &fromV = _vertices[fromVidx];
        if (fromV.classIdx >= 0)
            continue;

        fromV.classIdx = (int16_t) newClassIdx;
        const uint16_t fromKruskalVertexInsideClassIdx = fromV.getOrCreateKruskalVertexInsideClassIdx();

        const auto s = (uint16_t) fromV.edges.size();
        for (uint16_t i = 0; i < s; ++i) {
            const auto e = fromV.edges[i];
            auto &toV = _vertices[e.to];

            if (toV.dist < 0) {
                toV.dist = (int16_t) (neededDist + 1);
                result.push_back(e.to);
            }
            else if (toV.dist == neededDist) {
                if (toV.classIdx < 0) {
                    _kruskalEdgesInsideClass[_kruskalEdgesInsideClassNum++] = KruskalEdge{
                        fromKruskalVertexInsideClassIdx,
                        toV.getOrCreateKruskalVertexInsideClassIdx(),
                        e.weight
                    };
                    bfsQueue.push(e.to);
                }
            }
            else if (toV.dist == neededDist - 1)
                _kruskalEdgesBetweenClasses[_kruskalEdgesBetweenClassesNum++] = KruskalEdge{(uint16_t) toV.classIdx, newClassIdx, e.weight};
        }
    }

    return result;
}

void fillClassesKruskalDataAndComputeSumClassesMinSpanWeight(const uint16_t centerVertexIdx, long long &sumClassesMinSpanWeightOut) {
    _vertices[centerVertexIdx].dist = 0;
    std::vector<Class> currentUnion, nextUnion({{centerVertexIdx}});

    int16_t currentDist = 0;
    while (!nextUnion.empty()) {
        currentUnion = std::move(nextUnion);
        nextUnion.clear();

        for (auto &cl : currentUnion) {
            for (const uint16_t nextUnionVertexIdx : cl) {
                if (_vertices[nextUnionVertexIdx].classIdx >= 0)
                    continue;

                _kruskalVerticesInsideClassNum = 0;
                _kruskalEdgesInsideClassNum = 0;

                auto newClass(fillClassKruskalDataAndBuildClass(nextUnionVertexIdx, currentDist));

                if (_kruskalEdgesInsideClassNum > 0) {
                    kruskal(
                            _kruskalVerticesInsideClass, _kruskalVerticesInsideClassNum,
                            _kruskalEdgesInsideClass, _kruskalEdgesInsideClassNum,
                            sumClassesMinSpanWeightOut
                    );
                }

                if (!newClass.empty())
                    nextUnion.push_back(std::move(newClass));
            }
        }

        ++currentDist;
    }
}

bool solve(const uint16_t centerVertexIdx, long long &result) {
    _kruskalClassesNum = 0;
    _kruskalEdgesBetweenClassesNum = 0;

    fillClassesKruskalDataAndComputeSumClassesMinSpanWeight(centerVertexIdx, result);

    if (_kruskalEdgesBetweenClassesNum > 0) {
        // we can also eliminate same edges here and remain only edges with the lowest weight, but I'm tired :(
        // it is enough for performance to pass all tests though
        return kruskal(
                _kruskalClasses, _kruskalClassesNum,
                _kruskalEdgesBetweenClasses, _kruskalEdgesBetweenClassesNum,
                result
        );
    }

    return true;
}

int main() {
    uint16_t edgesNum;
    (void) scanf("%hd %hd", &_verticesNum, &edgesNum);

    uint16_t fromVert, toVert;
    int32_t weight;
    for (uint16_t i = 0; i < edgesNum; ++i) {
        (void) scanf("%hd %hd %d", &fromVert, &toVert, &weight);
        --fromVert, --toVert;
        _vertices[fromVert].edges.emplace_back(toVert, weight);
        _vertices[toVert].edges.emplace_back(fromVert, weight);
    }

    long long minCascadingSpanTreeWeight = std::numeric_limits<long long>::max();
    for (uint16_t i = 0; i < _verticesNum; ++i) {
        long long res = 0;
        if (solve(i, res) && res < minCascadingSpanTreeWeight)
            minCascadingSpanTreeWeight = res;

        // clear vertex variables needed for computations
        for (uint16_t j = 0; j < _verticesNum; ++j)
            _vertices[j].reset();
    }

    printf("%lld\n", minCascadingSpanTreeWeight);
    return 0;
}
