#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
#pragma once

struct song_part {
  std::string name;
  std::vector<std::string> paras;
};

// the database will not store the song parts, but the application
// will need them for displaying different parts of songs
// so we don't need to separate them for storage
struct song {
  uint64_t id; // the sqlite rowid
  uint64_t background_id;
  std::string name;
  std::string body; // body of the song, verse, chorus, etc.
  // std::vector<song_part> parts;
  std::string progression;

  std::string copyright_info;
};
