
#include <iostream>
#include <sqlite3.h>
#include <stdlib.h>

#pragma once

int init_and_open_db(char *, sqlite3 **);

int saveSong(song input_song, char * db_name);
