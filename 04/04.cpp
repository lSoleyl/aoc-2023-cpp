#include <iostream>
#include <fstream>
#include <string_view>
#include <set>
#include <algorithm>
#include <numeric>
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

    auto winningNumbers = regex::iter(std::string_view(match[2].first, match[2].second), numberRegex)
      | std::views::transform([](auto& match) { return std::stoi(match[0].str()); })
      | std::ranges::to<std::set>();

    auto ownNumbers = regex::iter(std::string_view(match[3].first, match[4].second), numberRegex)
      | std::views::transform([](auto& match) { return std::stoi(match[0].str()); })
      | std::ranges::to<std::vector>();

    matchCount = std::ranges::count_if(ownNumbers, [&](int number) { return winningNumbers.contains(number); });
  }


  int value() const {
    return matchCount ? 1 << (matchCount - 1) : 0;
  }


  int cardNumber;
  int matchCount; // we only need the match count for both parts
  int copiesForCard = 0; // the number of copies we receive for this card
};


int main()
{
  common::Time t;

  int part1 = 0;
  std::vector<Card> cards;

  // Part 1
  for (auto line : stream::lines(std::ifstream("input.txt"))) {
    cards.emplace_back(std::move(line));
    part1 += cards.back().value();
  }

  // Part 2: set the number of copies by iterating backwards (important to avoid costly recursion and reevaluation)
  for (int i = cards.size() - 1; i >= 0; --i) {
    auto& card = cards[i];
    // Add the count of immediately following copies to the count (std::min to ensure we don't copy past the last card)
    auto followingCopies = std::min(card.matchCount, (static_cast<int>(cards.size()) - 1) - i);
    card.copiesForCard += followingCopies;

    // Now for each of the following copies add up their copy value fo this card, because we will evaluate them too
    card.copiesForCard += std::ranges::fold_left(
      std::views::iota(0, followingCopies) | std::views::transform([&](int offset) { return cards[i + offset + 1].copiesForCard; }),
      0, std::plus{}
    );
  }

  // Count up the copiesForCard (+1 for the card itself)
  auto part2 = std::ranges::fold_left(cards | std::views::transform([](const Card& card) { return card.copiesForCard + 1; }), 0, std::plus{});


  std::cout << "Part 1: " << part1 << "\n"; // 21213
  std::cout << "Part 2: " << part2 << "\n"; // 8549735
  std::cout << t;
}