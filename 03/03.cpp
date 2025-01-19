#include <iostream>
#include <fstream>

#include <common/time.hpp>
#include <common/field.hpp>

struct Engine : public Field {
  Engine(std::istream&& source) : Field(source) {}

  // Returns true if the given position has a non-number symbol != '.' adjacent
  bool hasAdjacentSymbol(Vector pos) const {
    for (auto direction : Vector::AllDirections()) {
      if (auto value = at(pos + direction)) {
        if (!std::isdigit(*value) && *value != '.') {
          return true;
        }
      }
    }
    return false;
  }
};



int main() {
  common::Time t;
  int part1 = 0;
  int part2 = 0;

  Engine engine(std::ifstream("input.txt"));
  for (auto row : engine.rows()) {
    bool hasSymbol = false;
    int currentNumber = 0;

    // We need the iterator here to get the current position
    for (auto it = row.begin(), end = row.end(); it != end; ++it) {
      char symbol = *it;
      if (std::isdigit(symbol)) {
        currentNumber = currentNumber * 10 + (symbol - 0x30);
        if (!hasSymbol && engine.hasAdjacentSymbol(it.pos)) {
          hasSymbol = true;
        }
      } else if (currentNumber) {
        // '.' or other symbol following a number
        if (hasSymbol) {
          part1 += currentNumber;
        }
        // reset for next number
        currentNumber = 0;
        hasSymbol = false;
      }
    }

    // Check whether the row ended with a number
    if (currentNumber && hasSymbol) {
      part1 += currentNumber;
    }
  }
  
  std::cout << "Part 1: " << part1 << "\n"; // 528819
  std::cout << t;
}