

#include <regex>
#include <ranges>
#include <string_view>


namespace regex {
  template<typename Range>
  auto iter(const Range& searchString, const std::regex& regex) {
    using iterator_type = decltype(std::begin(searchString));
    return std::ranges::subrange {
      std::regex_iterator<iterator_type>(std::begin(searchString), std::end(searchString), regex),
      std::regex_iterator<iterator_type>()
    };
  }
}