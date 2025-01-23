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

  int rangeToNumber(Vector pos, Vector end, Vector direction = Vector::Right) const {
    int number = 0;
    for (; pos != end; pos += direction) {
      number = number * 10 + ((*this)[pos] - 0x30);
    }
    return number;
  }

  // Find the first position in the given direction, which is not a valid position or not a digit
  Vector getNumberEndPos(Vector startPos, Vector direction) {
    return std::ranges::find_if(rangeFromPositionAndDirection(startPos, direction), [](char ch) { return !std::isdigit(ch); }).pos;
  }

  std::optional<int> findGearRatio(Vector pos) {
    // We must collect all surrounding numbers and only return the result if we find exactly two.. but how to restrict the search?
    std::vector<int> numbers;

    // First check left of the gear
    if (std::isdigit(at(pos + Vector::Left, '.'))) {
      // Find the start position of the number
      auto startPos = getNumberEndPos(pos + Vector::Left, Vector::Left) + Vector::Right;
      numbers.push_back(rangeToNumber(startPos, pos));
    }

    // Now check right of gear
    if (std::isdigit(at(pos + Vector::Right, '.'))) {
      // Find the end position of the number
      auto endPos = getNumberEndPos(pos + Vector::Right, Vector::Right);
      numbers.push_back(rangeToNumber(pos + Vector::Right, endPos));
    }

    // We can have one or two numbers above/below
    auto topPositions = std::vector { pos + Vector::UpLeft, pos + Vector::Up, pos + Vector::UpRight };
    auto topLine = topPositions
      | std::views::transform([=](auto& pos) { return !!std::isdigit(at(pos, '.')); })
      | std::ranges::to<std::vector>();

    // I had to use vector<bool>, because std::isdigit() returned 4 instead of 1, so I couldn't check for equality
    if (topLine == std::vector {true, false, true}) {
      // The only constellation in which we have 2 distinct numbers on top
      auto startPos = getNumberEndPos(pos + Vector::UpLeft, Vector::Left) + Vector::Right;
      numbers.push_back(rangeToNumber(startPos, pos + Vector::Up));

      auto endPos = getNumberEndPos(pos + Vector::UpRight, Vector::Right);
      numbers.push_back(rangeToNumber(pos + Vector::UpRight, endPos));
    } else {
      // at most one number on top
      auto topPos = std::ranges::find_if(topPositions, [=](const Vector& pos) { return std::isdigit(at(pos, '.')); });
      if (topPos != topPositions.end()) {
        // exactly one number on top of the gear (find end in both directions)
        auto startPos = getNumberEndPos(*topPos, Vector::Left) + Vector::Right;
        auto endPos = getNumberEndPos(*topPos, Vector::Right);
        numbers.push_back(rangeToNumber(startPos, endPos));
      }
    }

    // now the same for below
    auto belowPositions = std::vector{ pos + Vector::DownLeft, pos + Vector::Down, pos + Vector::DownRight };
    auto belowLine = belowPositions
      | std::views::transform([=](auto& pos) { return !!std::isdigit(at(pos, '.')); })
      | std::ranges::to<std::vector>();

    if (belowLine == std::vector { true, false, true }) {
      // The only constellation in which we have 2 distinct numbers below
      auto startPos = getNumberEndPos(pos + Vector::DownLeft, Vector::Left) + Vector::Right;
      numbers.push_back(rangeToNumber(startPos, pos + Vector::Down));

      auto endPos = getNumberEndPos(pos + Vector::DownRight, Vector::Right);
      numbers.push_back(rangeToNumber(pos + Vector::DownRight, endPos));
    } else {
      // at most one number on top
      auto belowPos = std::ranges::find_if(belowPositions, [=](const Vector& pos) { return std::isdigit(at(pos, '.')); });
      if (belowPos != belowPositions.end()) {
        // exactly one number on top of the gear (find end in both directions)
        auto startPos = getNumberEndPos(*belowPos, Vector::Left) + Vector::Right;
        auto endPos = getNumberEndPos(*belowPos, Vector::Right);
        numbers.push_back(rangeToNumber(startPos, endPos));
      }
    }

    return numbers.size() == 2 ? std::optional(numbers[0] * numbers[1]) : std::nullopt;
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


  // Part 2: Start by searching for all '*' symbols and then search for all adjacent numbers
  for (size_t offset = engine.findOffset('*'); offset != std::numeric_limits<size_t>::max(); offset = engine.findOffset('*', offset+1)) {
    auto pos = engine.fromOffset(offset);
    if (auto ratio = engine.findGearRatio(pos)) {
      part2 += *ratio;
    }
  }


  
  std::cout << "Part 1: " << part1 << "\n"; // 528819
  std::cout << "Part 2: " << part2 << "\n"; // 80403602
  std::cout << t;
}