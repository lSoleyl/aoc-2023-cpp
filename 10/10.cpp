#include <cassert>
#include <unordered_set>

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


struct Loop {
  std::vector<Vector> positions;
  int clockWiseness = 0; // positive if loop spins clockwise, negative if counter clockwise

  Vector rotateInwards(Vector direction) const {
    return clockWiseness > 0 ? direction.rotateCW() : direction.rotateCCW();
  }
};


struct PipeField : FieldT<Tile> {
  PipeField(std::istream&& input) : FieldT(std::move(input)) {}

  Vector getStartPos() const {
    return fromOffset(findOffset(Kind::Start));
  }


  Loop findLoop(Vector startPos) {
    // We will only check 
    for (auto direction : Vector::AllSimpleDirections()) {
      if (auto loop = calcLoop(startPos, direction)) {
        return *loop;
      }
    }

    assert(false); // no loop?
    return {};
  }


  /** Finds the loop when starting in the given direction
   */
  std::optional<Loop> calcLoop(Vector startPos, Vector direction) const {
    Loop loop;
    loop.positions.push_back(startPos);
    for (Vector pos = startPos + direction; pos != startPos; pos += direction) {
      loop.positions.push_back(pos);
      if (auto nextTile = at(pos)) {
        // A valid tile
        if (auto nextDirection = nextTile->getExit(direction * -1)) {
          // Connected in the correct direction
          if (nextDirection == direction.rotateCW()) {
            ++loop.clockWiseness;
          } else if (nextDirection == direction.rotateCCW()) {
            --loop.clockWiseness;
          }

          direction = *nextDirection;
        } else {
          return std::nullopt;
        }
      } else {
        return std::nullopt; // left the field
      }
    }
    return loop;
  }

  // Part 2
  int countEnclosedFields(const Loop& loop) const {
    // We already determined the clockwiseness of the loop and we must now
    // follow the loop one more time and at each segment go in the direction of the inside of the loop
    // (the clockwiseness direction) and collect all fields in a set, which are in that direction before
    // touching another field that belongs to the loop itself.
    std::unordered_set<Vector> loopFields(loop.positions.begin(), loop.positions.end());
    std::unordered_set<Vector> enclosedFields;

    Vector lastDirection = Vector::Zero;
    Vector lastPosition = *loop.positions.begin();
    for (auto it = loop.positions.begin() + 1, end = loop.positions.end(); it != end; ++it) {
      auto position = *it;
      auto direction = position - lastPosition;

      if (direction == lastDirection) {
        // We entered this field in the same direction as we entered the previous one, which means that the previous one must 
        // have been a straight pipe. Turn the direction vector in clockwiseness direction and search for enclosed 
        collectEnclosedInDirection(lastPosition, loop.rotateInwards(direction), loopFields, enclosedFields);
      } else if (loop.rotateInwards(direction) == lastDirection) {
        // We rotated outwards of the loop, so we have a cornering piece with two edges facing the inside of the loop, we must check both directions
        collectEnclosedInDirection(lastPosition, loop.rotateInwards(direction), loopFields, enclosedFields);
        collectEnclosedInDirection(lastPosition, direction * -1, loopFields, enclosedFields); // *-1 = rotate inwards twice
      }

      lastPosition = position;
      lastDirection = direction;
    }

    return static_cast<int>(enclosedFields.size());
  }


  void collectEnclosedInDirection(Vector startPos, Vector direction, const std::unordered_set<Vector>& loopFields, std::unordered_set<Vector>& enclosedFields) const {
    for (auto position = startPos + direction; !loopFields.contains(position); position += direction) {
      assert(validPosition(position)); // we cannot actually leave the loop/field if the search in the clockwiseness direction
      enclosedFields.insert(position);
    }
  }

};


int main() {
  common::Time t;

  PipeField field(task::input());
  
  auto startPos = field.getStartPos();
  auto loop = field.findLoop(startPos);

  int part1 = (loop.positions.size() + 1) / 2; // round up
  int part2 = field.countEnclosedFields(loop);


  std::cout << "Part 1: " << part1 << "\n";
  std::cout << "Part 2: " << part2 << "\n";
  std::cout << t;

}