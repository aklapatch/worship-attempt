#include <iostream>
#include<sstream>
#include <sqlite3.h>
#include <stdlib.h>
#include "structs.h"

static int callback(void *notUsed, int argc, char **argv, char **colname) {
  for (int i = 0; i < argc; ++i) {
    std::cerr << colname[i] << " = " << (argv[i] ? argv[i] : "NULL") << "\n";
  }
  std::cerr << "\n";
  return 0;
}

// we will use the build in rowid that sqlite has instead of 
// making rowid's by ourselves. That should save me the work
// of making row id's by myself 
// opens out_db and leaves it open for the program
int init_and_open_db(char *fname, sqlite3 **out_db) {
  int rc = sqlite3_open(fname, out_db);
  if (rc) {
    std::cerr << "Failed to open " << fname << "\n";
    sqlite3_close(*out_db);
    return 1;
  }
  char *errmsg = NULL;

  const char *picture_sql_cmd = "create table if not exists pictures("
" name text,data blob);";

  rc = sqlite3_exec(*out_db, picture_sql_cmd, callback, 0, &errmsg);
  if (rc != SQLITE_OK) {
    std::cerr << "SQL Error " << __LINE__ << " " << errmsg << "\n";
    sqlite3_free(errmsg);
    return 2;
  }
  const char *sched_sql_cmd = "create table if not exists schedules("
                              "name TEXT"
                              ");";

  rc = sqlite3_exec(*out_db, sched_sql_cmd, callback, 0, &errmsg);
  if (rc != SQLITE_OK) {
    std::cerr << "SQL Error " << __LINE__ << errmsg << "\n";
    sqlite3_free(errmsg);
    return 2;
  }

  const char *song_sql_command =
      "create table if not exists songs("
      "title TEXT,"
      "body TEXT,"
      "progression TEXT,"
      "background_id INTEGER, "
	  "copyright_info TEXT,"
      "FOREIGN KEY(background_id) REFERENCES pictures(id)"
      ");";
  std::cout << song_sql_command << "\n";
  rc = sqlite3_exec(*out_db, song_sql_command, callback, 0, &errmsg);
  if (rc != SQLITE_OK) {
    std::cerr << "SQL Error " << __LINE__ << errmsg << "\n";
    sqlite3_free(errmsg);
    return 2;
  }
  const char *sched_song_link_sql_cmd =
      "create table if not exists sched_song_links("
      "song_id INTEGER,"
      "sched_id INTEGER,"
      "FOREIGN KEY(song_id) REFERENCES songs(id),"
      "FOREIGN KEY(sched_id) REFERENCES schedules(id)"
      ");";
  rc = sqlite3_exec(*out_db, sched_song_link_sql_cmd, callback, 0, &errmsg);
  if (rc != SQLITE_OK) {
    std::cerr << "SQL Error " << errmsg << "\n";
    sqlite3_free(errmsg);
    return 2;
  }
  return 0;
}
/*
      "id INTEGER PRIMARY KEY ASC,"
      "title TEXT,"
      "body TEXT,"
      "progression TEXT,"
      "background_id INTEGER, "
	  "copyright_info TEXT,"
      "FOREIGN KEY(background_id) REFERENCES pictures(id)"
	  */
// opens the db and saves the song to it.
int saveSong(song input_song, char * db_name){
  
  // open db
  sqlite3 *db;
  int rc = sqlite3_open(db_name, &db);
	
  if (rc) {
    std::cerr << "Failed to open " << db_name << "\n";
    sqlite3_close(db);
    return 1;
  }
  //convert values to one sql query 
  std::stringstream ss;
  
  ss << "insert or replace into songs values(";
  
  // the caller of this function makes sure that there is a name
  ss << "'" <<  input_song.name << "',"; 

  if (input_song.body.size() == 0){
	ss << "'" <<  input_song.body << "'";
  }
  else {
	ss << "NULL";
  }
  ss << ",";

  // TODO: add song bacground storage of some sort
  if (input_song.progression.size() == 0){
	ss << "'"<<  input_song.progression << "'";
  }
  else {
	ss << "NULL";
  }
  ss << ",NULL,";
  
  if (input_song.copyright_info.size() == 0){
	ss << "'" <<  input_song.copyright_info << "'";
  }
  else {
	ss << "NULL";
  }
  ss <<");";

  std::string sql = ss.str();
  std::cerr << "sql insert = " << sql << "\n";
  char * errmsg = NULL;
  
  rc = sqlite3_exec(db, sql.c_str(), callback, 0 ,&errmsg);
  std::cerr << sql << "\n";
  if (rc != SQLITE_OK){
	std::cerr << "SQL Error in " <<__FILE__ << ":" << __LINE__ << "\n" <<  errmsg << "\n";
	sqlite3_free(errmsg);
  sqlite3_close(db);
	return 2;
	}
  sqlite3_close(db);
  return 0;
}
  
