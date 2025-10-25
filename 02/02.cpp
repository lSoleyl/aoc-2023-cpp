#include <iostream>
#include <fstream>
#include <regex>

#include <common/stream.hpp>
#include <common/regex.hpp>
#include <common/split.hpp>
#include <common/time.hpp>
#include <common/task.hpp>

std::regex cubeRegex("([0-9]+) ((red)|(green)|(blue))");

struct Cubes {
  Cubes(int r = 0, int g = 0, int b = 0) : r(r), g(g), b(b) {}
  Cubes(std::string_view gamePart) : Cubes() {
    for (auto cube : regex::iter(gamePart, cubeRegex)) {
      auto matchStr = cube.str();
      if (cube[3].matched) { // red
        r = std::stoi(cube[1].str());
      } else if (cube[4].matched) { // green
        g = std::stoi(cube[1].str());
      } else { // blue
        b = std::stoi(cube[1].str());
      }
    }
  }

  bool operator<(const Cubes& other) const { return r < other.r && g < other.g && b < other.b; }
  bool operator<=(const Cubes& other) const { return r <= other.r && g <= other.g && b <= other.b; }
  Cubes max(const Cubes& other) const {
    return Cubes(std::max(r, other.r), std::max(g, other.g), std::max(b, other.b));
  }

  int power() const { return r * b * g; }

  int r, g, b;
};

std::regex gameRegex("^Game ([0-9]+): (.*)$");

struct Game {
  Game(const std::string& gameLine) {
    std::smatch match;
    std::regex_match(gameLine, match, gameRegex);
    id = std::stoi(match[1].str());
    for (auto cubesString : common::split(match[2].str(), "; ")) {
      cubes.push_back(Cubes(cubesString));
    }
  }

  int id;
  std::vector<Cubes> cubes;
};



int main()
{
  common::Time t;
  const Cubes LIMITS(12, 13, 14);

  int part1 = 0;
  int part2 = 0;
  for (auto gameStr : stream::lines(task::input())) {
    Game game(gameStr);
    if (std::ranges::all_of(game.cubes, [&](const Cubes& cubes) { return cubes <= LIMITS; })) {
      part1 += game.id;
    }

    part2 += std::ranges::fold_left(game.cubes, Cubes(), [](const Cubes& a, const Cubes& b) { return a.max(b); }).power();
  }

  std::cout << "Part 1: " << part1 << "\n";
  std::cout << "Part 2: " << part2 << "\n";
  std::cout << t;
}
