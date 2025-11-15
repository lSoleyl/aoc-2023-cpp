#include <cassert>

#include <common/time.hpp>
#include <common/task.hpp>
#include <common/field.hpp>
#include <common/vector.hpp>

enum class Kind : char {
  UpDown = '|',
  LeftRight = '-',
  UpRight = 'L',
  UpLeft = 'J',
  DownLeft = '7',
  DownRight = 'F',
  Ground = '.',
  Start = 'S'
};


struct Tile {
  Tile(char ch) : kind(static_cast<Kind>(ch)) {}
  Tile(Kind kind) : kind(kind) {}

  /** Returns the exit to take if we follow the pipe segment from the place we entered
   *  Returns none if we cannot enter the segment from this direction, or this is the ground or start segment
   */
  std::optional<Vector> getExit(const Vector& enteredFrom) const {
    auto vectors = connections();
    if (enteredFrom == vectors.first) {
      return vectors.second;
    } else if (enteredFrom == vectors.second) {
      return vectors.first;
    } else {
      return std::nullopt;
    }
  }

  bool operator==(const Tile& other) const {
    return kind == other.kind;
  }

  // Returns the pipe segment's connections (ground and startstate have 0 connnections for simplicity)
  std::pair<Vector, Vector> connections() const {
    switch (kind) {
      case Kind::UpDown: return std::make_pair(Vector::Up, Vector::Down);
      case Kind::UpRight: return std::make_pair(Vector::Up, Vector::Right);
      case Kind::UpLeft: return std::make_pair(Vector::Up, Vector::Left);
      case Kind::LeftRight: return std::make_pair(Vector::Left, Vector::Right);
      case Kind::DownRight: return std::make_pair(Vector::Down, Vector::Right);
      case Kind::DownLeft: return std::make_pair(Vector::Down, Vector::Left);
      default: return std::make_pair(Vector::Zero, Vector::Zero);
    }
  }

  Kind kind;
};


struct PipeField : FieldT<Tile> {
  PipeField(std::istream&& input) : FieldT(std::move(input)) {}

  Vector getStartPos() const {
    return fromOffset(findOffset(Kind::Start));
  }


  int findLoopLength(Vector startPos) const {
    for (auto direction : Vector::AllSimpleDirections()) {
      if (auto length = calcLoopLength(startPos, direction)) {
        return *length;
      }
    }

    assert(false); // no loop?
    return -1;
  }


  /** Finds the loop when starting in the given direction
   */
  std::optional<int> calcLoopLength(Vector startPos, Vector direction) const {
    int length = 1;
    for (Vector pos = startPos + direction; pos != startPos; pos += direction, ++length) {
      if (auto nextTile = at(pos)) {
        // A valid tile
        if (auto nextDirection = nextTile->getExit(direction * -1)) {
          // Connected in the correct direction
          direction = *nextDirection;
        } else {
          return std::nullopt;
        }
      } else {
        return std::nullopt; // left the field
      }
    }
    return length;
  }
};


int main() {
  common::Time t;

  PipeField field(task::input());
  
  auto startPos = field.getStartPos();
  int part1 = (field.findLoopLength(startPos) + 1) / 2; // round up

  int part2 = 0;


  std::cout << "Part 1: " << part1 << "\n";
  std::cout << "Part 2: " << part2 << "\n";
  std::cout << t;

}