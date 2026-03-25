#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstdint>

class Digraph {
private:
    struct Node {
        int value;
        size_t node_id;
        std::vector<size_t> incoming;
        std::vector<size_t> outgoing;
        bool active;

        Node(int val, size_t node_id)
            : value(val), node_id(node_id), active(true) {}
    };

    std::vector<Node> nodes;
    size_t nextNodeId;

    int findNodeIndex(size_t nodeId) const {
        for (size_t i = 0; i < nodes.size(); ++i) {
            if (nodes[i].active && nodes[i].node_id == nodeId) {
                return static_cast<int>(i);
            }            
        }
        return -1;
    }

    bool containsNode(size_t nodeId) const {
        for (size_t i = 0; i < nodes.size(); ++i) {
            if (nodes[i].active && nodes[i].node_id == nodeId) {
                return true;
            }
        }
        return false;        
    }

    void copyFrom(const Digraph& other) {
        nodes.clear();
        nextNodeId = other.nextNodeId;

        std::vector<size_t> indexMap(other.nodes.size(), SIZE_MAX);

        for (size_t i = 0; i < other.nodes.size(); ++i) {
            if (!other.nodes[i].active) { continue; }

            size_t new_index = nodes.size();
            nodes.emplace_back(other.nodes[i].value, other.nodes[i].node_id);
            nodes[new_index].active = true;
            indexMap[i] = new_index;
        }

        for (size_t i = 0; i < other.nodes.size(); ++i) {
            if (!other.nodes[i].active) { continue; }
            if (indexMap[i] == SIZE_MAX) { continue; }

            size_t from_idx = indexMap[i];
            for (size_t j = 0; j < other.nodes[i].outgoing.size(); ++j) {
                size_t descendant_idx = other.nodes[i].outgoing[j];
                if (!other.nodes[descendant_idx].active) { continue; }
                if (indexMap[descendant_idx] == SIZE_MAX) { continue; }

                size_t to_idx = indexMap[descendant_idx];
                nodes[from_idx].outgoing.push_back(to_idx);
                nodes[to_idx].incoming.push_back(from_idx);
            }
        }
    }

    void writeGraphToDotFile(std::ofstream& file, const std::string& graphname) const {
        file << "digraph " << graphname << " {\n";
        file << "    rankdir=LR;\n";
        file << "    node [shape=circle];\n";

        for (size_t i = 0; i < nodes.size(); ++i) {
            if (nodes[i].active) {
                file << "    n" << i << " [label=\"" << nodes[i].value << " (id = " << nodes[i].node_id << ")\"];\n";
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
    
public:
    Digraph(): nextNodeId(0) {}

    Digraph(const Digraph& other): nextNodeId(0) {
        copyFrom(other);
    }

    Digraph& operator=(const Digraph& other) {
        if (this != &other) {
            copyFrom(other);
        }
        return *this;
    }

    Digraph(Digraph&& other) noexcept 
        : nodes(std::move(other.nodes)), nextNodeId(other.nextNodeId) {
            other.nextNodeId = 0;
            other.nodes.clear();
        }
    
    Digraph& operator=(Digraph&& other) noexcept {
        if (this != &other) {
            nodes = std::move(other.nodes);
            nextNodeId = other.nextNodeId;
            other.nextNodeId = 0;
            other.nodes.clear();
        }
        return *this;
    }

    ~Digraph() = default;



    size_t addNode(int value) {
        nodes.emplace_back(value, nextNodeId++);
        return nodes.size() - 1;
    }

    void addEdge(size_t from, size_t to) {
        if (from >= nodes.size() || to >= nodes.size()) { return; }
        if (!nodes[from].active || !nodes[to].active) { return; }

        for (size_t i = 0; i < nodes[from].outgoing.size(); ++i) {
            if (nodes[from].outgoing[i] == to) {
                return;
            }
        }
        
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
                auto& in = nodes[out_idx].incoming;
                in.erase(std::remove(in.begin(), in.end(), index), in.end());
            }
        }

        for (size_t i = 0; i < node.incoming.size(); ++i) {
            size_t in_idx = node.incoming[i];
            if (in_idx < nodes.size()) {
                auto& out = nodes[in_idx].outgoing;
                out.erase(std::remove(out.begin(), out.end(), index), out.end());
            }
        }

        node.incoming.clear();
        node.outgoing.clear();
        node.active = false;
    }

    Digraph uniteWith(const Digraph& other) const {
        Digraph result(other);

        std::vector<size_t> ids;
        std::vector<size_t> values;
        std::vector<size_t> indices;

        for (size_t i = 0; i < result.nodes.size(); ++i) {
            if (!other.nodes[i].active) { continue; }
            ids.push_back(result.nodes[i].node_id);
            values.push_back(result.nodes[i].value);
            indices.push_back(i);
        }

        for (size_t i = 0; i < nodes.size(); ++i) {
            if (!nodes[i].active) { continue; }

            size_t this_nodeId = nodes[i].node_id;
            int    this_value  = nodes[i].value;
            bool found = false;
            for (size_t j = 0; j < ids.size(); ++j) {
                if (ids[j] == this_nodeId && values[j] == this_value) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                size_t new_idx = result.addNode(nodes[i].value);
                result.nodes[new_idx].node_id = this_nodeId;
                ids.push_back(this_nodeId);
                values.push_back(this_value);
                indices.push_back(new_idx);
            }
        }

        auto getIndex = [&](size_t nodeId) -> size_t {
            for (size_t j = 0; j < ids.size(); ++j) {
                if (ids[j] == nodeId) { return indices[j]; }
            }
            return 0;
        };

        for (size_t i = 0; i < nodes.size(); ++i) {
            if (!nodes[i].active) { continue; }

            size_t from_idx = getIndex(nodes[i].node_id);
            for (size_t j = 0; j < nodes[i].outgoing.size(); ++j) {
                size_t descendant_idx = nodes[i].outgoing[j];
                if (!nodes[descendant_idx].active) { continue; }

                size_t to_idx = getIndex(nodes[descendant_idx].node_id);
                result.addEdge(from_idx, descendant_idx);
            }
        }

        return result;
    }    

    Digraph intersectWith(const Digraph& other) const {
        Digraph result;

        std::vector<size_t> commonIds;
        for (size_t i = 0; i < nodes.size(); ++i) {
            if (!nodes[i].active) { continue; }

            size_t this_id = nodes[i].node_id;
            if (other.containsNode(this_id)) {
                commonIds.push_back(this_id);
            }
        }

        std::vector<size_t> ids;
        std::vector<size_t> indices;

        for (size_t i = 0; i < commonIds.size(); ++i) {
            size_t commonId = commonIds[i];

            int value = 0;
            for (size_t j = 0; j < nodes.size(); ++j) {
                if (nodes[j].active && nodes[j].node_id == commonId) {
                    value = nodes[j].value;
                    break;
                }
            }

            size_t new_idx = result.addNode(value);
            result.nodes[new_idx].node_id = commonId;
            ids.push_back(commonId);
            indices.push_back(new_idx);
        }

        auto getIndex = [&](size_t nodeId) -> size_t {
            for (size_t j = 0; j < ids.size(); ++j) {
                if (ids[j] == nodeId) {
                    return indices[j];
                }
            }
            return 0;
        };

        // add common edges
        for (size_t i = 0; i < ids.size(); ++i) {
            size_t fromId  = ids[i];
            size_t fromIdx = indices[i];

            int this_index = findNodeIndex(fromId);
            if (this_index == -1) {continue;}

            int other_index = other.findNodeIndex(fromId);
            if (other_index == -1) {continue;}

            for (size_t j = 0; j < nodes[this_index].outgoing.size(); ++j) {
                size_t this_desc_idx = nodes[this_index].outgoing[j];
                if(!nodes[this_desc_idx].active) { continue; }

                size_t desc_id = nodes[this_desc_idx].node_id;

                bool edgeInOther = false;
                for (size_t k = 0; k < other.nodes[other_index].outgoing.size(); ++k) {
                    size_t other_desc_idx = other.nodes[other_index].outgoing[k];
                    if (other.nodes[other_desc_idx].active &&
                        other.nodes[other_desc_idx].node_id == desc_id) {
                        edgeInOther = true;
                        break;
                    }
                }

                if (edgeInOther) {
                    size_t toIdx = getIndex(desc_id);
                    result.addEdge(fromIdx, toIdx);
                }
            }
        }

        return result;
    }

    void ToDotFile(const std::string& filename, const std::string& graphname = "Digraph") const {
        std::ofstream file(filename);
        writeGraphToDotFile(file, graphname);
    }

    static void writeMultipleGraphs(const std::string& filename, 
                                    const std::vector<const Digraph*>& graphs,
                                    const std::vector<std::string>& names) {
        if (graphs.size() != names.size()) {
            std::cerr << "Error: number of graphs and number of names doesn't match!\n";
            return;
        }

        std::ofstream file(filename);
        file << "digraph All {\n";
        file << "    rankdir=TB;\n";
        file << "    compound=true;\n\n";        

        for (size_t i = 0; i < graphs.size(); ++i) {
            file << "    subgraph cluster_" << i << " {\n";
            file << "        label=\"" << names[i] << "\";\n";
            file << "        style=bold;\n";

            const Digraph* graph = graphs[i];

            for (size_t j = 0; j < graph->nodes.size(); ++j) {
                if (graph->nodes[j].active) {
                    file << "        g" << i << "_n" << j << " [label=\"" << graph->nodes[j].value 
                         << " (id = " << graph->nodes[j].node_id  << ")\"];\n";
                }
            }

            for (size_t j = 0; j < graph->nodes.size(); ++j) {
                if (!graph->nodes[j].active) { continue; }
                for (size_t k = 0; k < graph->nodes[j].outgoing.size(); ++k) {
                    file << "        g" << i << "_n" << j << " -> g" << i << "_n" << graph->nodes[j].outgoing[k] << ";\n";
                }
            }

            file << "    }\n\n";
        }

        file << "}\n";
        file.close();
    }
};

int main() {
    Digraph g1;
    
    size_t a1 = g1.addNode(1);
    size_t b1 = g1.addNode(2);
    size_t c1 = g1.addNode(3);
    
    g1.addEdge(a1, b1);
    g1.addEdge(b1, c1);
    g1.addEdge(a1, c1);

    Digraph g2;
    
    size_t a2 = g2.addNode(99);
    size_t b2 = g2.addNode(2);
    size_t c2 = g2.addNode(3);

    g2.addEdge(b2, b2);
    g2.addEdge(c2, b2);    

    Digraph united = g1.uniteWith(g2);
    Digraph intersected = g1.intersectWith(g2);

    std::vector<const Digraph*> graphs = {&g1, &g2, &united, &intersected};
    std::vector<std::string> names = {"G1", "G2", "United", "Intersected"};

    Digraph::writeMultipleGraphs("all_digraphs.dot", graphs, names);

    // dot -Tpng myGraph.dot -o myGraph.png
    // dot -Tpdf myGraph.dot -o myGraph.pdf
    
    return 0;
}