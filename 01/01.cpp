#include <fstream>
#include <chrono>
#include <regex>
#include <map>

#include <common/stream.hpp>
#include <common/time.hpp>

std::map<std::string, int> digitValue = {
  { "0", 0 },
  { "1", 1 },
  { "2", 2 },
  { "3", 3 },
  { "4", 4 },
  { "5", 5 },
  { "6", 6 },
  { "7", 7 },
  { "8", 8 },
  { "9", 9 },
  { "one", 1 },
  { "two", 2 },
  { "three", 3 },
  { "four", 4 },
  { "five", 5 },
  { "six", 6 },
  { "seven", 7 },
  { "eight", 8 },
  { "nine", 9 }
};



int main() {
  common::Time t;

  std::ifstream input("input.txt");

  std::regex firstDigitRegex("([0-9]|one|two|three|four|five|six|seven|eight|nine)");
  std::regex lastDigitRegex(".*([0-9]|one|two|three|four|five|six|seven|eight|nine)");
  int calibrationValue = 0;
  int part2 = 0;

  for (auto line : stream::lines(input)) {
    int digit1 = line[line.find_first_of("0123456789")] - 0x30;
    int digit2 = line[line.find_last_of("0123456789")] - 0x30;
    calibrationValue += digit1 * 10 + digit2;

    // We need two seperate regexes for this and we cannot simply iterate over the 
    // matches as the numbers may overlap i.e. "eighthree"
    std::smatch match;
    std::regex_search(line, match, firstDigitRegex);
    digit1 = digitValue[match[1].str()];
    std::regex_search(line, match, lastDigitRegex);
    digit2 = digitValue[match[1].str()];
    part2 += digit1 * 10 + digit2;
  }
  

  std::cout << "Part 1: " << calibrationValue << "\n"; // 54644
  std::cout << "Part 2: " << part2 << "\n"; // 53348
  std::cout << t;
}
