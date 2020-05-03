
#include <iostream>
#include<sstream>
#include <sqlite3.h>
#include <stdlib.h>
#include<string.h>
#include<stdio.h>
#include "structs.h"
#include <iostream>
#include <sqlite3.h>
#include <stdlib.h>
#include<vector>

#pragma once

typedef enum{
  SUCCESS = 0,
  SONG_EXISTS,
  SQL_ERROR,
  OPEN_FAILURE,
  INIT_VAL, // the error value you init an error variable with
} db_error;

db_error init_and_open_db(char *, sqlite3 **);

db_error saveSong(song input_song, char * db_name);

db_error readSongs(sqlite3*, std::vector<song>&, std::vector<char*> &);

db_error deleteSong(char*fname, std::string song_name);

// this will open the database
db_error saveImg(std::vector<unsigned char>, std::string);
