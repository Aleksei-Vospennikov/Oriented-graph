#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstdint>

template <typename T>
struct Vertex {
        T value;
        size_t vertexId;        

        Vertex(T val, size_t vertexId)
            : value(val), vertexId(vertexId) {}
    };

template <typename T>
class Digraph {
private: 
    std::vector<Vertex<T>> vertices;
    std::vector<std::vector<char>> adjMatrix;
    size_t nextVertexId;    

    void copyFrom(const Digraph& other) {
        vertices.clear();
        nextVertexId = other.nextVertexId;

        vertices = other.vertices;
        adjMatrix = other.adjMatrix;
    }

    void writeGraphToDotFile(std::ofstream& file, const std::string& graphname) const {
        file << "digraph " << graphname << " {\n";
        file << "    rankdir=LR;\n";
        file << "    node [shape=record];\n";

        for (size_t i = 0; i < vertices.size(); ++i) {
            file << "    n" << i << " [label=\"{{ " << vertices[i].value << " | " << vertices[i].vertexId << " }}\"];\n";            
        }

        for (size_t i = 0; i < vertices.size(); ++i) {
            for (size_t j = 0; j < vertices.size(); ++j) {
                if (adjMatrix[i][j] == 1) {
                    file << "    n" << i << " -> n" << j << ";\n";
                }
            }
        }

        file << "}\n";
    }
    
public:
    Digraph(): nextVertexId(0) {}

    Digraph(const Digraph& other): nextVertexId(0) {
        copyFrom(other);
    }

    Digraph& operator=(const Digraph& other) {
        if (this != &other) {
            copyFrom(other);
        }
        return *this;
    }

    Digraph(Digraph&& other) noexcept 
        : vertices(std::move(other.vertices)), adjMatrix(std::move(other.adjMatrix)), nextVertexId(other.nextVertexId) {
            other.nextVertexId = 0;
            other.vertices.clear();
            other.adjMatrix.clear();
    }
    
    Digraph& operator=(Digraph&& other) noexcept {
        if (this != &other) {
            vertices = std::move(other.vertices);
            adjMatrix = std::move(other.adjMatrix);
            nextVertexId = other.nextVertexId;
            other.nextVertexId = 0;
            other.vertices.clear();
            other.adjMatrix.clear();
        }
        return *this;
    }

    ~Digraph() = default;

    // Base methods for class Digraph

    size_t addVertex(T value) {
        int n = static_cast<int>(vertices.size());
        vertices.emplace_back(value, nextVertexId++);

        for (int i = 0; i < n; ++i) {
            adjMatrix[i].push_back(0);
        }

        adjMatrix.push_back(std::vector<char>(n + 1, 0));    
        
        return nextVertexId - 1;
    }

    void addEdge(size_t from, size_t to) {
        if (!hasVertex(from) || !hasVertex(to)) { return; }

        int fromIndex = getVertexIndex(from);
        int toIndex   = getVertexIndex(to);

        adjMatrix[from][to] = 1;
    }

    void removeEdge(size_t from, size_t to) {
        if (!hasVertex(from) || !hasVertex(to)) { return; }
        
        int fromId = getVertexIndex(from);
        int toId   = getVertexIndex(to);

        adjMatrix[fromId][toId] = 0;
    }    

    void removeVertex(size_t id) {
        if (!hasVertex(id)) { return; }
        
        size_t vertexIndex = static_cast<size_t>(getVertexIndex(id));
        size_t nVertices = vertices.size();

        for (size_t i = vertexIndex; i < nVertices - 1; ++i) {
            vertices[i] = std::move(vertices[i + 1]);
        }
        vertices.pop_back();

        for (size_t i = vertexIndex; i < nVertices - 1; ++i) {
            adjMatrix[i] = std::move(adjMatrix[i + 1]);
        }
        adjMatrix.pop_back();

        for (size_t i = 0; i < nVertices - 1; ++i) {
            for (size_t j = vertexIndex; j < nVertices - 1; ++j) {
                adjMatrix[i][j] = std::move(adjMatrix[i][j + 1]);
            }
            adjMatrix[i].pop_back();
        }
    }

    void toDotFile(const std::string& filename, const std::string& graphname = "Digraph") const {
        std::ofstream file(filename);
        writeGraphToDotFile(file, graphname);

    }

    bool hasVertex(size_t id) const {
        return getVertexIndex(id) != -1;
    }

    int getVertexIndex(size_t id) const {
        for (size_t i = 0; i < vertices.size(); ++i) {
            if (vertices[i].vertexId == id) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    const std::vector<Vertex<T>>& getAllVertices() const {
        return vertices;
    }

    const std::vector<std::vector<char>>& getAdjMatrix() const {
        return adjMatrix;
    }
};

struct Edge {
    size_t fromId;
    size_t toId;
};

template <typename T>
class Subgraph {
private:    
    const Digraph<T>& original;
    std::vector<Vertex<T>> activeVertices;
    std::vector<std::vector<char>> activeAdjMatrix;

    int getVertexIndex(size_t id) const {
        for (size_t i = 0; i < activeVertices.size(); ++i) {
            if (activeVertices[i].vertexId == id) {
                return static_cast<int>(i);
            }
        }

        return -1;
    }

    bool hasVertex(size_t id) const {
        return getVertexIndex(id) != -1;
    }

public:    
    explicit Subgraph(const Digraph<T>& original) 
        : original(original), activeVertices(original.getAllVertices()), activeAdjMatrix(original.getAdjMatrix()) {}
    
    Subgraph(const Digraph<T>& original, 
             const std::vector<size_t>& subsetVerticesIds, 
             const std::vector<Edge>& subsetEdges) : original(original) {
        
        activeAdjMatrix.resize(subsetVerticesIds.size(), std::vector<char>(subsetVerticesIds.size(), 0));

        for (size_t i = 0; i < subsetVerticesIds.size(); ++i) {
            int originIndex = original.getVertexIndex(subsetVerticesIds[i]);
            if (originIndex != -1) {
                activeVertices.push_back(original.getAllVertices()[originIndex]);
            }
        }

        for (size_t i = 0; i < subsetEdges.size(); ++i) {
            if (!original.hasVertex(subsetEdges[i].fromId) || !original.hasVertex(subsetEdges[i].toId)) {
                continue;
            }

            int fromIndex = getVertexIndex(subsetEdges[i].fromId);
            int toIndex   = getVertexIndex(subsetEdges[i].toId);

            if (fromIndex == -1 || toIndex == -1) {
                continue;
            }

            int originalFrom = original.getVertexIndex(subsetEdges[i].fromId);
            int originalTo   = original.getVertexIndex(subsetEdges[i].toId);

            activeAdjMatrix[fromIndex][toIndex] = original.getAdjMatrix()[originalFrom][originalTo];
        }
    }

    const std::vector<Vertex<T>>& getActiveVertices() const {
        return activeVertices;
    } 

    const std::vector<std::vector<char>>& getAdjMatrix() const {
        return activeAdjMatrix;
    }
};


template <typename T>
static void writeMultipleGraphs(const std::string& filename, 
                                const std::vector<const Subgraph<T>*>& graphs, 
                                const std::vector<std::string>& names) {
        if (graphs.size() != names.size()) {
            std::cerr << "Error: number of graphs and number of names doesn't match!\n";
            return;
        }

        std::ofstream file(filename);
        file << "digraph All {\n";
        file << "    rankdir=LR;\n";
        file << "    compound=true;\n";
        file << "    node [shape=record, style=\"rounded,filled\"];\n\n";

        for (size_t i = 0; i < graphs.size(); ++i) {
            file << "    subgraph cluster_" << i <<" {\n";
            file << "        label=\"" << names[i] << "\";\n";
            file << "        style=bold;\n";

            const Subgraph<T>* subgraph = graphs[i];

            for (size_t j = 0; j < subgraph->getActiveVertices().size(); ++j) {
                file << "        g" << i << "_n" << j << " [label=\"{ " << subgraph->getActiveVertices()[j].vertexId 
                     << " | " << subgraph->getActiveVertices()[j].value 
                     << " }\", fillcolor=\"antiquewhite1\"];\n";
            }

            for (size_t j = 0; j < subgraph->getActiveVertices().size(); ++j) {
                for (size_t k = 0; k < subgraph->getActiveVertices().size(); ++k) {
                    if (subgraph->getAdjMatrix()[j][k] == 1) {
                        file << "        g" << i << "_n" << j << " -> g" << i << "_n" << k << ";\n";
                    }
                }
            }

            file << "    }\n\n";
        }

        file << "}\n";
        file.close();
    }


int main() {
    Digraph<int> g;
    
    size_t a = g.addVertex(10);
    size_t b = g.addVertex(20);
    size_t c = g.addVertex(30);
    size_t d = g.addVertex(40);
    size_t e = g.addVertex(50);
    size_t f = g.addVertex(60);
    size_t h = g.addVertex(70);

    g.addEdge(a, b);
    g.addEdge(a, c);
    g.addEdge(a, d);
    g.addEdge(b, d);
    g.addEdge(b, e);
    g.addEdge(c, d);
    g.addEdge(d, a);
    g.addEdge(d, e);
    g.addEdge(d, f);
    g.addEdge(d, h);
    g.addEdge(e, c);
    g.addEdge(h, f);

    Subgraph<int> original(g);

    std::vector<size_t> subv = {a, b, c, d, f};
    std::vector<Edge>   sube = {{a, b}, {a, c}, {a, d}, {c, d}, {d, f}};
    Subgraph<int> subg(g, subv, sube);

    std::vector<const Subgraph<int>*> graphs = {&original, &subg};
    std::vector<std::string> names = {"Original", "Subgraph"};
    writeMultipleGraphs("all.dot", graphs, names);

    // dot -Tpng myGraph.dot -o myGraph.png
    // dot -Tpdf myGraph.dot -o myGraph.pdf
    
    return 0;
}