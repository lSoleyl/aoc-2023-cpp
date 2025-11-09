#include <unordered_map>


#include <common/time.hpp>
#include <common/task.hpp>
#include <common/stream.hpp>
#include <common/regex.hpp>



struct Node {
  std::string id;

  Node* left;
  Node* right;


  Node* get(char instruction) {
    // only 'L' and 'R' allowed:
    return (instruction == 'L') ? left : right;
  }

  static void loadNodes(std::istream& input) {
    std::regex lineRegex("^([A-Z]+) = \\(([A-Z]+), ([A-Z]+)\\)$");
    for (auto line : stream::lines(input)) {
      if (auto result = regex::match(line, lineRegex)) {
        auto& node = nodes[result[1]]; // implicitly create the node if not already done
        node.id = result[1];
        node.left = &nodes[result[2]]; // implicitly create the referenced node if not already done
        node.right = &nodes[result[3]];
      }
    }
  }

  static std::unordered_map<std::string, Node> nodes;
};

std::unordered_map<std::string, Node> Node::nodes;


int main() {
  common::Time t;

  auto input = task::input();
  auto instructions = stream::line(input);
  stream::line(input); // ignore second line
  Node::loadNodes(input); // load all nodes


  int part1 = 0;
  auto node = &Node::nodes["AAA"];
  for (int instructionIx = 0; node->id != "ZZZ"; instructionIx = (instructionIx+1) % instructions.length()) {
    node = node->get(instructions[instructionIx]);
    ++part1;
  }

  
  int part2 = 0;
 

  std::cout << "Part 1: " << part1 << "\n";
  std::cout << "Part 2: " << part2 << "\n";
  std::cout << t;

}