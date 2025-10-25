#include <map>
#include <algorithm>

#include <common/time.hpp>
#include <common/task.hpp>
#include <common/stream.hpp>


std::vector<char> cardOrder = { '2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K', 'A' };

int cardValue(char card) {
  return std::distance(cardOrder.begin(), std::find(cardOrder.begin(), cardOrder.end(), card));
}



struct Hand {
  std::array<char, 5> cards;
  int bid;
  enum Strength : int {
    HighCard,
    OnePair,
    TwoPair,
    ThreeOfAKind,
    FullHouse,
    FourOfAKind,
    FiveOfAKind
  } strength;


  Strength calcStrength(bool maxStrength = false) {
    // To determine the hand, simply count occurrences:
    std::vector<std::pair<char, int>> occurrences;

    int jokers = 0;
    for (auto card : cards) {
      if (maxStrength && card == 'J') {
        // Count jokers, but don't enter them jokers into occurrence map, we will redistribute them anyway
        ++jokers;
        continue; 
      }

      auto pos = std::find_if(occurrences.begin(), occurrences.end(), [=](auto& entry) { return entry.first == card; });
      if (pos != occurrences.end()) {
        ++pos->second;
      } else {
        occurrences.emplace_back(card, 1);
      }
    }

    // Now sort by occurrences
    std::sort(occurrences.begin(), occurrences.end(), [](auto& a, auto& b) { return a.second > b.second; });
    
    auto first = occurrences.size() > 0 ? occurrences[0].second : 0;
    auto second = occurrences.size() > 1 ? occurrences[1].second : 0;

    // always add the jokers to the highest number of occurences that way we maximize the strength of this hand
    // In case we only had Jokers, we simply add 5 to 0 and still receive a FiveOfAKind
    first += jokers;

    // Use the highest two occurrences to determine the hand strength
    switch (first) {
      case 5: return Strength::FiveOfAKind;
      case 4: return Strength::FourOfAKind;
      case 3: return (second == 2) ? Strength::FullHouse : Strength::ThreeOfAKind;
      case 2: return (second == 2) ? Strength::TwoPair : Strength::OnePair;
      default: return Strength::HighCard;
    }
  }

  // Compare hands by strength, then by first, second, ... card
  bool operator<(const Hand& other) const {
    if (strength != other.strength) {
      return strength < other.strength;
    }

    return std::lexicographical_compare(cards.begin(), cards.end(), other.cards.begin(), other.cards.end(), [](char cardA, char cardB) {
      return cardValue(cardA) < cardValue(cardB);
    });
  }
};


std::istream& operator>>(std::istream& in, Hand& hand) {
  for (auto& card : hand.cards) {
    in >> card;
  }
  in >> hand.bid;
  hand.strength = hand.calcStrength();
  return in;
}

std::vector<Hand> readHands(std::istream&& input) {
  Hand hand;
  std::vector<Hand> hands;
  do {
    input >> hand;
    if (input) {
      hands.push_back(hand);
    }
  } while (input);
  return hands;
}


int main() {
  common::Time t;

  auto hands = readHands(task::input());
  std::sort(hands.begin(), hands.end());


  // Part 1
  int64_t part1 = 0;
  int rank = 0;
  for (auto& hand : hands) {
    part1 += hand.bid * ++rank;
  }



  // Part 2
  
  // Update strength according to new rules
  for (auto& hand : hands) {
    hand.strength = hand.calcStrength(true);
  }

  // Set new card order for sorting
  cardOrder = { 'J', '2', '3', '4', '5', '6', '7', '8', '9', 'T', 'Q', 'K', 'A' };

  // Sort again according to new hand strengths and card ordering
  std::sort(hands.begin(), hands.end());


  // Part 1
  int64_t part2 = 0;
  rank = 0;
  for (auto& hand : hands) {
    part2 += hand.bid * ++rank;
  }

  std::cout << "Part 1: " << part1 << "\n";
  std::cout << "Part 2: " << part2 << "\n";
  std::cout << t;

}