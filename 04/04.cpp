#include <iostream>
#include <fstream>
#include <string_view>
#include <set>
#include <algorithm>
#include <ranges>
#include <regex>

#include <common/time.hpp>
#include <common/stream.hpp>
#include <common/regex.hpp>

std::regex lineRegex("^Card +([0-9]+): (.*?)\\| (.*)$");
std::regex numberRegex("[0-9]+");

struct Card {
  Card(std::string line) {
    std::smatch match;
    std::regex_match(line, match, lineRegex);
    cardNumber = std::stoi(match[1].str());
    
    winningNumbers = regex::iter(std::string_view(match[2].first, match[2].second), numberRegex)
      | std::views::transform([](auto& match) { return std::stoi(match[0].str()); })
      | std::ranges::to<std::set>();

    ownNumbers = regex::iter(std::string_view(match[3].first, match[4].second), numberRegex)
      | std::views::transform([](auto& match) { return std::stoi(match[0].str()); })
      | std::ranges::to<std::vector>();
  }

  int value() const {
    int wonCount = std::ranges::count_if(ownNumbers, [=](int number) { return winningNumbers.contains(number); });
    return wonCount ? 1 << (wonCount - 1) : 0;
  }


  int cardNumber;
  std::set<int> winningNumbers;
  std::vector<int> ownNumbers;
};




int main()
{
  common::Time t;

  int part1 = 0;
  for (auto line : stream::lines(std::ifstream("input.txt"))) {
    Card card(std::move(line));
    part1 += card.value();
  }


  std::cout << "Part1: " << part1 << "\n"; // 21213
  std::cout << t;
}