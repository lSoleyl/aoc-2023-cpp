#include <unordered_map>
#include <unordered_set>
#include <cassert>
#include <numeric>

#include <common/time.hpp>
#include <common/task.hpp>
#include <common/stream.hpp>
#include <common/regex.hpp>
#include <common/hash.hpp>



struct Node {
  std::string id;

  Node* left;
  Node* right;


  Node* get(char instruction) {
    // only 'L' and 'R' allowed:
    return (instruction == 'L') ? left : right;
  }

  bool isGhostEndNode() const {
    return id.back() == 'Z';
  }

  static void loadNodes(std::istream& input) {
    std::regex lineRegex("^([A-Z0-9]+) = \\(([A-Z0-9]+), ([A-Z0-9]+)\\)$");
    for (auto line : stream::lines(input)) {
      if (auto result = regex::match(line, lineRegex)) {
        auto& node = nodes[result[1]]; // implicitly create the node if not already done
        node.id = result[1];
        node.left = &nodes[result[2]]; // implicitly create the referenced node if not already done
        node.right = &nodes[result[3]];
      }
    }
  }

  // Returns all ghost starting nodes as a vector
  static std::vector<Node*> ghostStartNodes() {
    std::vector<Node*> result;
    for (auto& [id, node] : nodes) {
      if (id.back() == 'A') {
        result.push_back(&node);
      }
    }
    return result;
  }

  static std::unordered_map<std::string, Node> nodes;
};

std::unordered_map<std::string, Node> Node::nodes;


struct LoopEntry {
  LoopEntry(Node* node, size_t index, int instructionIndex) : node(node), index(index), instructionIndex(instructionIndex) {}

  Node* node;
  size_t index;
  int instructionIndex;
  

  bool operator==(const LoopEntry& other) const { return node == other.node && instructionIndex == other.instructionIndex; }
  bool operator!=(const LoopEntry& other) const { return node != other.node || instructionIndex != other.instructionIndex; }
};

template<>
struct std::hash<LoopEntry> {
  size_t operator()(const LoopEntry& entry) const { 
    return hash_all(entry.node, entry.instructionIndex);
  }
};


struct NodeLoop {
  size_t head;    // Start of a loop
  size_t period;  // Period of a loop
  size_t zOffset; // Offset of node ending in 'Z' relative to loop head (only 1 in my input)


  static NodeLoop calculate(Node* node, const std::string& instructions) {
    // Try to determine the loop head and the loop length
    std::unordered_set<LoopEntry> loopSet;
    std::vector<size_t> zIndicies;
    std::vector<Node*> visitedNodes; // kinda expensive for the large input, but is necessary to determine the "real" period for sample2.txt

    NodeLoop loop;
    for (size_t index = 0, instruction = 0;; ++index, ++instruction) {
      if (instruction >= instructions.size()) {
        instruction = 0;
      }
      // Keep track of all visited nodes in visitation order
      visitedNodes.push_back(node);
      
      // Insert the node with the current index to check whether we already visited the same node a the same instruction index
      auto [pos, inserted] = loopSet.emplace(node, index, instruction);
      if (!inserted) {
        // Insertion failed for the first time, so we found our loop!
        loop.head = pos->index;
        loop.period = index - pos->index;
        break;
      }

      if (node->isGhostEndNode()) {
        // found an end node -> write it down
        zIndicies.push_back(index);
      }

      // Fetch the next node
      node = node->get(instructions[instruction]);
    }


    // Check for a smaller period inside the detected loop by looking for the start node inside the loop
    // and checking whether the nodes repeat until the end
    loop.reducePeriod(std::move(visitedNodes));



    // Remove all 'Z' indicies we found before our loop starts
    std::erase_if(zIndicies, [&](size_t index) { return index < loop.head || index >= loop.head + loop.period; });
    
    // The following assert was true for my and the sample input and simplifies calculations
    assert(zIndicies.size() == 1);

    // Calculate zOffsets as relative to the loop head
    loop.zOffset = zIndicies[0] - loop.head;

    // The following holds true both for the "sample2.txt" and "input.txt" and allows us to actually directly calculate the result
    // This assertion does not hold true for "sample.txt", so the part2 calculation will fail for "sample.txt"
    assert(loop.head + loop.zOffset == loop.period);
    return loop;
  }



  /** This method takes all visited nodes and tries to find a smaller period than the current one
   *  This step is only needed to solve for the "sample2.txt" data.
   */
  void reducePeriod(std::vector<Node*> visitedNodes) {
    visitedNodes.erase(visitedNodes.begin(), visitedNodes.begin() + head);

    // Now find all occurences of the loop's start node inside the visited nodes and try to trace a loop until the end.
    auto startNode = visitedNodes[0];
    for (auto startPos = std::find(visitedNodes.begin() + 1, visitedNodes.end(), startNode); startPos != visitedNodes.end(); startPos = std::find(startPos+1, visitedNodes.end(), startNode)) {
      if (std::equal(startPos, visitedNodes.end(), visitedNodes.begin())) {
        // We found the shortest period for the given sequence of node visits
        period = std::distance(visitedNodes.begin(), startPos);
        return;
      }
    }

    // If we are here, then we found no smaller period
  }
};


std::ostream& operator<<(std::ostream& out, const NodeLoop& loop) {
  return out << "(head=" << loop.head << ", period=" << loop.period << ", zOffset=" << loop.zOffset << ")";
}


int main() {
  common::Time t;

  auto input = task::input();
  auto instructions = stream::line(input);
  stream::line(input); // ignore second line
  Node::loadNodes(input); // load all nodes


  int part1 = 0;
  if (Node::nodes.count("AAA")) { // <- only here to not crash when running it with "sample2.txt"
    auto node = &Node::nodes["AAA"];
    for (int instructionIx = 0; node->id != "ZZZ"; instructionIx = (instructionIx+1) % instructions.length()) {
      node = node->get(instructions[instructionIx]);
      ++part1;
    }
  }

  


  // Part 2 is actually pretty challenging:
  // The pure brute force approach works for the sample, but leads nowhere for the input with 50.000.000.000 steps tried so far.
  // 
  // My input consists of 730 Nodes and a chain of 270 instructions.
  // Since each node has two outputs any set of instructions will eventually end up in a loop.
  // The longest possible loop would have the length 730*270 = 197.100, which we exceeded by far with out brute force attempt.
  // 
  // So the approach is to find all start nodes and for each start node determine, when the loop starts and what the loop period is.
  // For example something like A -> B -> C -> Z -> Z would have a loop head of 3 and a period of 1 because it loops into one state.
  // Then we will note down all 'Z' states inside the looping part.
  // 
  // We have 6 start states in our input, so that is doable and takes about 300ms in Debug with the current approach.
  // 
  // There are two observations from these results, which truely simplify the task at hand:
  // 1. for my input there is only exactly one 'Z' state in any such loop, which simplifies what follows.
  // 2. for my input head+zOffset equals period (which is not at all general, but simplifies the calculation by a LOT)
  // 
  // Thanks to the data loop layout (having the 'Z' node exactly at the end of the loop), we only have to take 
  // the least common multiple of all periods and we are done.

  
  // Find all start nodes
  auto nodes = Node::ghostStartNodes();

  // Calculate the loops
  std::vector<NodeLoop> loops;
  for (auto node : nodes) {
    loops.push_back(NodeLoop::calculate(node, instructions));
  }

  // Display the detected loops
  for (auto& loop : loops) {
    std::cout << loop << "\n";
  }

  // Calculate least common mulitple of all loop periods
  auto part2 = std::accumulate(loops.begin() + 1, loops.end(), loops[0].period, [](size_t total, const NodeLoop& loop) { return std::lcm(total, loop.period); });

  std::cout << "Part 1: " << part1 << "\n";
  std::cout << "Part 2: " << part2 << "\n";
  std::cout << t;

}