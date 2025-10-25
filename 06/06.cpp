#include <vector>
#include <numeric>

#include <common/time.hpp>
#include <common/task.hpp>
#include <common/stream.hpp>



struct Race {
  int64_t time;
  int64_t record;

  int64_t distance(int64_t buttonTime) {
    int64_t raceTime = std::min<int64_t>(time - buttonTime, 0);
    return raceTime * buttonTime /* here buttonTime is the speed */;
  }

  std::pair<int64_t, int64_t> minMaxValues() {
    // If we ignore negative speeds, then the above equation is:
    // d = (time - button) * button
    //   = time*button - button*button

    // If we want to figure out the time a button has been pressed to reach a specific distance(record), then we have
    // the following quadratic equation to solve:
    // 0 = - button*button + time*button - record
    //   = button^2 - time*button + record
    // 
    // button[1/2] = time/2 +- sqrt(time^2/4 - record)

    double root = std::sqrt(time * time / 4.0 - record);
    
    double maxD = time / 2.0 + root;
    double minD = time / 2.0 - root;
    
    // We must round away from the center (the best possible value) and then add 1 towards that center value
    // So if we end up with an exact value for the record, then we will move the time by 1 towards the center to 
    // get a time BETTER THAN the current record.
    // If we end up with a value between two integer numbers, then we just select the next integer number beating the record.
    int64_t max = static_cast<int>(std::ceil(maxD))  - 1;
    int64_t min = static_cast<int>(std::floor(minD)) + 1;

    return std::make_pair(min, max);
  }

  /** Returns the number of ways we have to beat this concrete record
   */
  int64_t calcNumOptions() {
    auto [minDuration, maxDuration] = minMaxValues();
    return maxDuration - minDuration + 1;
  }


  /** Merges two adjacent races together to 'fix the kerning issue'
   */
  static Race merge(const Race& first, const Race& second) {
    return Race {
      .time = concat(first.time, second.time),
      .record = concat(first.record, second.record)
    };
  }

private:
  /** Concatenates two numbers
   */
  static int64_t concat(int64_t a, int64_t b) {
    auto power10 = static_cast<int>(std::floor(std::log10(b))) + 1;
    return a * static_cast<int64_t>(std::pow(10, power10)) + b;
  }
};


std::vector<Race> parseInput(std::ifstream&& input) {
  std::istringstream times(stream::line(input));
  std::istringstream records(stream::line(input));

  // read header column
  std::string dummy;
  times >> dummy;
  records >> dummy;

  // Read the race dates
  std::vector<Race> races;
  while (times && records) {
    Race race;
    times >> race.time;
    records >> race.record;
    if (times && records) {
      races.push_back(race);
    }
  }

  return races;
}





int main() {
  common::Time t;


  auto races = parseInput(task::input());

  int64_t part1 = 1;
  for (auto& race : races) {
    part1 *= race.calcNumOptions();
  }


  auto fullRace = std::accumulate(races.begin() + 1, races.end(), *races.begin(), Race::merge);
  auto part2 = fullRace.calcNumOptions();


  std::cout << "Part 1: " << part1 << "\n";
  std::cout << "Part 2: " << part2 << "\n";
  std::cout << t;
}