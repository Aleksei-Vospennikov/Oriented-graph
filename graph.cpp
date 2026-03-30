#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstdint>

template <typename T>
class Digraph {
private:
    struct Vertex {
        T value;
        size_t vertexId;        

        Vertex(T val, size_t vertexId)
            : value(val), vertexId(vertexId) {}
    };

    std::vector<Vertex> vertices;
    std::vector<std::vector<char>> adjMatrix;
    size_t nextVertexId;

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

    void copyFrom(const Digraph& other) {
        vertices.clear();
        nextVertexId = other.nextVertexId;

        vertices = other.vertices;
        adjMatrix = other.adjMatrix;
    }

    void writeGraphToDotFile(std::ofstream& file, const std::string& graphname) const {
        file << "digraph " << graphname << " {\n";
        file << "    rankdir=LR;\n";
        file << "    node [shape=circle];\n";

        for (size_t i = 0; i < vertices.size(); ++i) {
            file << "    n" << i << " [label=\"" << vertices[i].value << " (id = " << vertices[i].vertexId << ")\"];\n";            
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
};

int main() {
    Digraph<int> g;
    
    size_t a = g.addVertex(1);
    size_t b = g.addVertex(2);
    size_t c = g.addVertex(3);

    g.addEdge(a, b);
    g.addEdge(b, c);
    g.addEdge(a, c);

    g.removeEdge(b, c);
    g.removeVertex(c);

    g.toDotFile("digraph.dot", "myDigraph");

    // dot -Tpng myGraph.dot -o myGraph.png
    // dot -Tpdf myGraph.dot -o myGraph.pdf
    
    return 0;
}