#include <cassert>
#include <algorithm>

#include <common/time.hpp>
#include <common/task.hpp>
#include <common/stream.hpp>


std::vector<int> parseLine(const std::string& line) {
  std::istringstream input(line);

  std::vector<int> numbers;
  do {
    int number;
    if (input >> number) {
      numbers.push_back(number);
    }
  } while (input);

  return numbers;
}

std::vector<int> adjacentDifference(const std::vector<int>& numbers) {
  assert(!numbers.empty());
  std::vector<int> difference;
  difference.reserve(numbers.size() - 1);
  for (auto pos = numbers.begin(), end = numbers.end() - 1; pos != end; ++pos) {
    difference.push_back(pos[1] - pos[0]);
  }
  return difference;
}

std::pair<int, int> calculatePreviousAndNext(const std::vector<int>& numbers) {
  auto difference = adjacentDifference(numbers);
  if (std::ranges::all_of(difference, [](int value) { return value == 0; })) {
    return std::make_pair(numbers.front(), numbers.back()); // + 0
  } else {
    auto [previous, next] = calculatePreviousAndNext(difference);
    return std::make_pair(numbers.front() - previous, numbers.back() + next);
  }
}



int main() {
  common::Time t;


  int part1 = 0;
  int part2 = 0;

  for (auto line : stream::lines(task::input())) {
    auto numbers = parseLine(line);
    auto [previous, next] = calculatePreviousAndNext(numbers);
    part1 += next;
    part2 += previous;
  }

  
  std::cout << "Part 1: " << part1 << "\n";
  std::cout << "Part 2: " << part2 << "\n";
  std::cout << t;

}