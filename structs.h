#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
#pragma once

struct song_part {
  std::vector<char> name, text;
};

struct song {
  std::vector<char> name;
  std::vector<song_part> parts;

  // copyright info
  std::vector<char> author, pub_company, pub_date;
};
