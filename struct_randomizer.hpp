#pragma once

#include <meta>

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

// ============================================================
// Configuration
// ============================================================
#ifndef RANDOM_LAYOUT_SEED
#error "RANDOM_LAYOUT_SEED must be defined. Ex: -DRANDOM_LAYOUT_SEED=0x44"
#endif

// Random padding inserted inbetween each field.
// Default 16
#ifndef RANDOM_LAYOUT_MAX_PADDING
#define RANDOM_LAYOUT_MAX_PADDING 16
#endif

namespace struct_randomizer {

template <std::size_t N> struct fixed_string {
  char data[N]{};

  consteval fixed_string(const char (&str)[N]) {
    for (std::size_t i = 0; i < N; ++i)
      data[i] = str[i];
  }

  consteval std::string_view view() const {
    return std::string_view{data, N - 1};
  }

};

template <std::size_t N> fixed_string(const char (&)[N]) -> fixed_string<N>;

consteval std::uint64_t fnv1a(std::string_view str) {
  std::uint64_t hash = 14695981039346656037ULL;

  for (char c : str) {
    hash ^= static_cast<unsigned char>(c);
    hash *= 1099511628211ULL;
  }

  return hash;
}

template <fixed_string Name, typename T> struct field {
  static consteval std::meta::info make() {
    return std::meta::data_member_spec(^^T, { .name = Name.view() });
  }

  static consteval std::uint64_t hash() { return fnv1a(Name.view()); }
};

template <std::size_t N> struct padding {
  static_assert(N > 0);
  std::byte bytes[N];
};

consteval std::uint64_t random_next(std::uint64_t &state) {
  state += 0x9E3779B97F4A7C15ULL;

  std::uint64_t z = state;
  z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
  z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;

  return z ^ (z >> 31);
}

template <typename... Fields>
consteval std::uint64_t fields_seed(std::uint64_t seed) {
  ((seed ^= Fields::hash() + 0x9E3779B97F4A7C15ULL + (seed << 6) + (seed >> 2)),
   ...);

  return seed;
}

consteval std::string make_padding_name(std::size_t index) {
  std::string result = "__random_layout_padding_";

  char buffer[32]{};
  std::size_t count = 0;

  do {
    buffer[count++] = static_cast<char>('0' + (index % 10));
    index /= 10;
  } while (index != 0);

  while (count > 0) {
    result.push_back(buffer[--count]);
  }

  return result;
}

consteval std::meta::info make_padding(std::size_t size, std::size_t index) {
  const std::meta::info padding_type =
      std::meta::substitute(^^padding, { std::meta::reflect_constant(size) });
  const std::string name = make_padding_name(index);
  return std::meta::data_member_spec(padding_type, {.name = name});
}

template <std::uint64_t BuildSeed, std::uint64_t GroupSalt, typename... Fields>
struct field_group {
  struct type;

  consteval {

    std::vector<std::meta::info> fields{Fields::make()...};
    std::uint64_t rng = fields_seed<Fields...>(BuildSeed ^ GroupSalt);

    for (std::size_t i = fields.size(); i > 1; --i) {
      const std::size_t j = static_cast<std::size_t>(random_next(rng) % i);

      const auto temp = fields[i - 1];

      fields[i - 1] = fields[j];
      fields[j] = temp;
    }

    std::vector<std::meta::info> members;
    members.reserve(fields.size() * 2 + 1);

    std::size_t padding_index = 0;

    for (const auto field : fields) {
      const std::size_t padding_size = static_cast<std::size_t>(
          random_next(rng) %
          (static_cast<std::size_t>(RANDOM_LAYOUT_MAX_PADDING) + 1));

      if (padding_size != 0) {
        members.push_back(make_padding(padding_size, padding_index++));
      }

      members.push_back(field);
    }

    const std::size_t tail_padding = static_cast<std::size_t>(
        random_next(rng) %
        (static_cast<std::size_t>(RANDOM_LAYOUT_MAX_PADDING) + 1));

    if (tail_padding != 0) {
      members.push_back(make_padding(tail_padding, padding_index++));
    }

    std::meta::define_aggregate(^^type, members);
  }
};

inline constexpr std::uint64_t public_salt = 0xA82F9D17C563B491ULL;
inline constexpr std::uint64_t protected_salt = 0x75D13AE891C42F67ULL;
inline constexpr std::uint64_t private_salt = 0xD4E20C619B8735A3ULL;

} // namespace struct_randomizer

#define F(type, name)                                                          \
  ::struct_randomizer::field<::struct_randomizer::fixed_string{#name}, type>

#define P_PUBLIC(...)                                                          \
  public ::struct_randomizer::field_group<                                     \
      static_cast<std::uint64_t>(RANDOM_LAYOUT_SEED),                          \
      ::struct_randomizer::public_salt, __VA_ARGS__>::type

#define P_PRIVATE(...)                                                         \
  private ::struct_randomizer::field_group<                                    \
      static_cast<std::uint64_t>(RANDOM_LAYOUT_SEED),                          \
      ::struct_randomizer::private_salt, __VA_ARGS__>::type

#define P_PROTECTED(...)                                                       \
  protected ::struct_randomizer::field_group<                                  \
      static_cast<std::uint64_t>(RANDOM_LAYOUT_SEED),                          \
      ::struct_randomizer::protected_salt, __VA_ARGS__>::type
