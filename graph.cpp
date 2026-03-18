#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>

class Digraph {
private:
    struct Node {
        int value;
        std::vector<size_t> incoming;
        std::vector<size_t> outgoing;
        bool active;

        Node(int val): value(val), active(true) {}
    };

    std::vector<Node> nodes;
    
public:
    Digraph() = default;
    
    size_t addNode(int value) {
        nodes.emplace_back(value);
        return nodes.size() - 1;
    }

    void addEdge(size_t from, size_t to) {
        if (from >= nodes.size() || to >= nodes.size()) { return; }
        if (!nodes[from].active || !nodes[to].active) { return; }

        auto& out = nodes[from].outgoing;
        if (std::find(out.begin(), out.end(), to) != out.end()) { return; }

        nodes[from].outgoing.push_back(to);
        nodes[to].incoming.push_back(from);
    }

    void removeEdge(size_t from, size_t to) {
        if (from >= nodes.size() || to >= nodes.size()) { return; }
        if (!nodes[from].active || !nodes[to].active) { return; }

        auto& out = nodes[from].outgoing;
        out.erase(std::remove(out.begin(), out.end(), to), out.end());

        auto& in = nodes[to].incoming;
        in.erase(std::remove(in.begin(), in.end(), from), in.end());
    }

    void removeNode(size_t index) {
        if (index >= nodes.size() || !nodes[index].active) { return; }

        Node& node = nodes[index];

        for (size_t i = 0; i < node.outgoing.size(); ++i) {
            size_t out_idx = node.outgoing[i];
            if (out_idx < nodes.size()) {
                removeEdge(index, out_idx);
            }
        }

        for (size_t i = 0; i < node.incoming.size(); ++i) {
            size_t in_idx = node.incoming[i];
            if (in_idx < nodes.size()) {
                removeEdge(in_idx, index);
            }
        }

        node.incoming.clear();
        node.outgoing.clear();
        node.active = false;
    }

    void GenerateDotFile(const std::string& filename, const std::string& graphname = "Digraph") {
        std::ofstream file(filename);
        file << "digraph " << graphname << " {\n";
        file << "    rankdir=LR;\n";
        file << "    node [shape=circle];\n";

        for (size_t i = 0; i < nodes.size(); ++i) {
            if (nodes[i].active) {
                file << "    n" << i << " [label=\"" << nodes[i].value << "\"];\n";
            }
        }

        for (size_t i = 0; i < nodes.size(); ++i) {
            if (!nodes[i].active) { continue; }
            for (size_t neighbour : nodes[i].outgoing) {
                file << "    n" << i << " -> n" << neighbour << ";\n";
            }
        }

        file << "}\n";
    }
};

int main() {
    Digraph graph;
    
    size_t a = graph.addNode(1);
    size_t b = graph.addNode(2);
    size_t c = graph.addNode(3);
    
    graph.addEdge(a, b);
    graph.addEdge(b, c);
    graph.addEdge(a, c);
    
    graph.GenerateDotFile("myDigraph.dot", "myDigraph");
    // dot -Tpng myGraph.dot -o myGraph.png
    // dot -Tpdf myGraph.dot -o myGraph.pdf
    
    return 0;
}