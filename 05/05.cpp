#include <iostream>
#include <fstream>
#include <string>
#include <ranges>
#include <cassert>
#include <limits>

#include <common/time.hpp>
#include <common/stream.hpp>
#include <common/regex.hpp>
#include <common/task.hpp>

struct Range {
  Range(int64_t begin, int64_t end) : begin(begin), end(end) {}
  int64_t begin, end; // begin; end as usually defined for iterators

  bool empty() const { return begin == end; }

  bool contains(int64_t value) const { return value >= begin && value < end; }

  bool overlaps(const Range& other) const { return contains(other.begin) || other.contains(begin); }

  /** Calculates the overlap range between this and other
   * @pre this.overlaps(other)
   */
  Range overlap(const Range& other) const { return Range(std::max(begin, other.begin), std::min(end, other.end)); }

  bool valid() const { return begin <= end; }

  Range operator+(int64_t offset) const { 
    Range result(begin + offset, end + offset); 
    assert(result.valid()); // we have to add some checks and special handling if this triggers
    return result;
  }

  Range operator-(int64_t offset) const {
    Range result(begin - offset, end - offset);
    assert(result.valid()); // we have to add some checks and special handling if this triggers
    return result;
  }
};


struct MapEntry {
  MapEntry(int64_t targetBegin, int64_t sourceBegin, int64_t size) : source(sourceBegin, sourceBegin+size), offset(targetBegin - sourceBegin) {}
  MapEntry(Range source, int64_t offset) : source(source), offset(offset) {}
  Range source; // source range
  int64_t offset; // offset to target range (target.begin - source.begin)

  Range mappedRange() const {
    // We should ensure that the resulting range is valid and no side overflows...
    // For now lets just hope this isn't the case
    return source + offset;
  }
};


std::regex rangeRegex("([0-9]+) ([0-9]+) ([0-9]+)");
struct Map {
  Map(std::string from, std::string to) : from(std::move(from)), to(std::move(to)) {}
  Map(std::string from, std::string to, std::istream& input) : from(std::move(from)), to(std::move(to)) {
    for (auto line : stream::lines(input)) {
      if (line.empty()) {
        break; // end of map section
      }

      auto match = regex::match(line, rangeRegex);
      map.emplace_back(std::stoll(match[1].str()), std::stoll(match[2].str()), std::stoll(match[3].str()));
    }

    // Finally the range entries
    normalizeMap();
  }

  /** Sorts the entries in the map and ensures that there is no gap in the ranges by entering new mapping entries
   */
  void normalizeMap() {
    std::sort(map.begin(), map.end(), [](const MapEntry& a, const MapEntry& b) { return a.source.begin < b.source.begin; });

    // by defining the end at max value, that value can never be part of the range, so this is the only value we don't support.
    Range remaining(0, std::numeric_limits<int64_t>::max());
    // Next index in map to check - we keep an index, because we will insert new entries, which will result in re-allocations
    // and thus will make tracking positions through iterators more difficult
    size_t mapIndex = 0; 
    while (!remaining.empty()) {
      // Find the first entry from the map, overlapping the remaining range and adjust the map ranges accordingly
      while (mapIndex < map.size() && !map[mapIndex].source.overlaps(remaining)) {
        ++mapIndex;
      }

      if (mapIndex == map.size()) {
        // No overlapping entry left and the remaining range is not empty -> 
        // add the whole remaining range into the map
        map.emplace_back(remaining, 0);
        remaining.begin = remaining.end; // set remaining range to empty range
      } else {
        auto overlapped = map[mapIndex]; // copy the original map entry out - a reference is too dangerous since we may cause reallocations
        
        // Now we must split up the ranges. Since we go small to big, we assume that remaining.begin will always be <= entry.begin
        if (remaining.begin < overlapped.source.begin) {
          // We have a small range before the overlap -> create a mapping entry before mapIndex
          map.emplace(map.begin() + mapIndex, Range(remaining.begin, overlapped.source.begin), 0);
          remaining.begin = overlapped.source.begin; // reduce remaining range accordingly
          ++mapIndex; // adjust index accordingly
        }

        // Now simply subtract the overlapped range from the remaining one since both
        // start at the same position
        remaining.begin = overlapped.source.end;
        ++mapIndex; // continue after the current mapping entry
      }
    }

    // Remove adjacent entries with same offset
    auto previous = map.begin();
    for (auto current = map.begin() + 1; current != map.end(); ++current, ++previous) {
      if (current->offset == previous->offset) {
        // combine both mapping entries into one
        previous->source.end = current->source.end;
        current = map.erase(current);
        // Decrement iterators to not skip the next entry
        --current;
        --previous;
      }
    }
  }


  // apply the map to the given value
  int64_t operator()(int64_t value) const {
    // Use binary search to find the MapEntry, whose source.end range is AFTER the value i.e. the range should include the value
    // We should always find a value
    auto pos = std::upper_bound(map.begin(), map.end(), value, [](int64_t value, const MapEntry& entry) { return value < entry.source.end; });
    assert(pos != map.end()); // would indicate an unnormalized map
    return value + pos->offset;
  }


  /** This will merge this map with the given other map by calculating the resulting mapping
   *  of first appling AB and then BC
   */
  static Map combine(const Map& AB, const Map& BC) {
    if (AB.to != BC.from) {
      throw std::exception("Invalid map merge");
    }

    Map AC(AB.from, BC.to);

    // We have AC = A -> B -> C
    //         AB = A -> B
    //         BC =      B -> C

    // Merge the ranges ... but how?
    for (auto& abEntry : AB.map) {
      auto bRange = abEntry.mappedRange();
      // Now find all overlapping ranges in other and create mapping entries
      // while reducing the midrange to an empty range
      auto bcPos = BC.map.begin();
      while (!bRange.empty()) {
        while (bcPos != BC.map.end() && !bcPos->source.overlaps(bRange)) {
          ++bcPos;
        }
        assert(bcPos != BC.map.end()); // since we have a total map, we must be able to map each possible value
        auto& bcEntry = *bcPos;
        auto bOverlap = bRange.overlap(bcEntry.source); // overlap in B
        assert(bOverlap.begin == bRange.begin); // the overlap should always be at the beginning of our midrange
        // We must translate back the overlap range from B into A (the AB source range) before we enter it into the result range
        auto aOverlap = bOverlap - abEntry.offset;
        auto acOffset = abEntry.offset + bcEntry.offset; // simply add both offsets to get the total offset
        AC.map.emplace_back(aOverlap, acOffset);
        bRange.begin = bOverlap.end; // We processed the whole overlap -> continue after it
      }
    }
    

    // Normalize resulting map as it is verly likely unsorted. (I would expect no source range holes though)
    AC.normalizeMap();
    return AC;
  }



  std::string from;
  std::string to;
  std::vector<MapEntry> map; // <- sorted ascending by sourceBegin
};

std::regex numberRegex("[0-9]+");
std::regex mapRegex("([a-z]+)-to-([a-z]+) map:$");

struct Almanac {
  Almanac(std::istream&& input) : combined("<INVALID>", "<INVALID>") {
    auto seedLine = stream::line(input);
    for (auto& match : regex::iter(seedLine, numberRegex)) {
      seeds.push_back(std::stoll(match[0].str()));
    }
    
    stream::line(input); // read empty line
    while (auto match = regex::match(stream::line(input), mapRegex)) {
      maps.emplace_back(match[1].str(), match[2].str(), input);
    }

    // Combine all maps into one complete map
    combined = Map::combine(maps[0], maps[1]);
    for (auto pos = maps.begin() + 2, end = maps.end(); pos != end; ++pos) {
      combined = Map::combine(combined, *pos);
    }
  }

  /** Returns the location value for a given seed (i.e) pass it through the combined map
   */
  int64_t getLocation(int64_t seedValue) const {
    return combined(seedValue);
  }

  int64_t minLocation() const {
    auto locations = seeds | std::views::transform([=](int64_t seed) { return getLocation(seed); }) | std::ranges::to<std::vector>();
    return std::ranges::min(locations);
  }

  int64_t minLocationForRange(Range range) const {
    // Simply check all overlapping ranges.
    auto minLocation = std::numeric_limits<int64_t>::max();
    for (auto& mapEntry : combined.map) {
      if (range.overlaps(mapEntry.source)) {
        // We only check .begin of the overlapped range, because we cannot get a smaller value later
        auto minMapped = range.overlap(mapEntry.source).begin + mapEntry.offset;
        minLocation = std::min(minLocation, minMapped);
      }
    }

    return minLocation;
  }

  int64_t minLocationForRanges() const {
    int64_t minLocation = std::numeric_limits<int64_t>::max();
    for (auto pos = seeds.begin(), end = seeds.end(); pos != end; pos += 2) {
      minLocation = std::min(minLocation, minLocationForRange(Range(pos[0], pos[0]+pos[1])));
    }
    return minLocation;
  }



  std::vector<int64_t> seeds;
  std::vector<Map> maps; // maps in declaration order
  Map combined; // all maps combined into one
};



int main()
{
  common::Time t;
  
  Almanac almanac(task::input());
  auto minLocation = almanac.minLocation();
  auto minRangeLocation = almanac.minLocationForRanges();

  std::cout << "Part 1: " << minLocation << "\n";
  std::cout << "Part 2: " << minRangeLocation << "\n";
  std::cout << t;
}
