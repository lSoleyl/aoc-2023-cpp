#include <iostream>
#include <fstream>
#include <string>
#include <ranges>

#include <common/time.hpp>
#include <common/stream.hpp>
#include <common/regex.hpp>

struct Range {
  Range(int64_t begin, int64_t end) : begin(begin), end(end) {}
  int64_t begin, end; // begin; end as usually defined for iterators
};


struct MapEntry {
  MapEntry(int64_t targetBegin, int64_t sourceBegin, int64_t size) : source(sourceBegin, sourceBegin+size), target(targetBegin, targetBegin+size) {}
  Range source, target; // source and target ranges (always same size)
};


std::regex rangeRegex("([0-9]+) ([0-9]+) ([0-9]+)");
struct Map {
  Map(std::string from, std::string to, std::istream& input) : from(std::move(from)), to(std::move(to)) {
    for (auto line : stream::lines(input)) {
      if (line.empty()) {
        break; // end of map section
      }

      auto match = regex::match(line, rangeRegex);
      map.emplace_back(std::stoll(match[1].str()), std::stoll(match[2].str()), std::stoll(match[3].str()));
    }

    // Finally the range entries
    std::sort(map.begin(), map.end(), [](const MapEntry& a, const MapEntry& b) { return a.source.begin < b.source.begin; });
  }



  // apply the map to the given value
  int64_t operator()(int64_t value) const {
    // Use binary search to find the MapEntry, whose source.end range is AFTER the value i.e. the range should include the value
    auto pos = std::upper_bound(map.begin(), map.end(), value, [](int64_t value, const MapEntry& entry) { return value < entry.source.end; });
    if (pos != map.end() && pos->source.begin <= value) {
      // return translated value
      return value - pos->source.begin + pos->target.begin;
    }
    return value; // otherwise this map defines no translation
  }

  std::string from;
  std::string to;
  std::vector<MapEntry> map; // <- sorted ascending by sourceBegin
};

std::regex numberRegex("[0-9]+");
std::regex mapRegex("([a-z]+)-to-([a-z]+) map:$");

struct Almanac {
  Almanac(std::istream&& input) {
    auto seedLine = stream::line(input);
    for (auto& match : regex::iter(seedLine, numberRegex)) {
      seeds.push_back(std::stoll(match[0].str()));
    }
    
    stream::line(input); // read empty line
    while (auto match = regex::match(stream::line(input), mapRegex)) {
      maps.emplace_back(match[1].str(), match[2].str(), input);
    }
  }

  /** Returns the location value for a given seed (i.e) pass it through all the maps
   */
  int64_t getLocation(int64_t seedValue) const {
    for (auto& map : maps) {
      seedValue = map(seedValue);
    }
    return seedValue;
  }

  int64_t minLocation() const {
    auto locations = seeds | std::views::transform([=](int64_t seed) { return getLocation(seed); }) | std::ranges::to<std::vector>();
    return std::ranges::min(locations);
  }

  std::vector<int64_t> seeds;
  std::vector<Map> maps; // maps in declaration order
};



int main()
{
  common::Time t;
  
  Almanac almanac(std::ifstream("input.txt"));
  auto minLocation = almanac.minLocation();

  std::cout << "Part 1: " << minLocation << std::endl;
  std::cout << t;
}
