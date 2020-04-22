#include <iostream>
#include<sstream>
#include <sqlite3.h>
#include <stdlib.h>
#include<string.h>
#include<stdio.h>
#include "structs.h"

#include"sqlite_routines.h"


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
db_error init_and_open_db(char *fname, sqlite3 **out_db) {
  int rc = sqlite3_open(fname, out_db);
  if (rc) {
    std::cerr << "Failed to open " << fname << "\n";
    sqlite3_close(*out_db);
    return OPEN_FAILURE;
  }
  char *errmsg = NULL;

  const char *picture_sql_cmd = "create table if not exists pictures("
" name text,data blob);";

  rc = sqlite3_exec(*out_db, picture_sql_cmd, callback, 0, &errmsg);
  if (rc != SQLITE_OK) {
    std::cerr << "SQL Error " << __LINE__ << " " << errmsg << "\n";
    sqlite3_free(errmsg);
    return SQL_ERROR;
  }
  const char *sched_sql_cmd = "create table if not exists schedules("
                              "name TEXT"
                              ");";

  rc = sqlite3_exec(*out_db, sched_sql_cmd, callback, 0, &errmsg);
  if (rc != SQLITE_OK) {
    std::cerr << "SQL Error " << __LINE__ << errmsg << "\n";
    sqlite3_free(errmsg);
    return SQL_ERROR;
  }

  const char *song_sql_command =
      "CREATE TABLE if not exists songs(title TEXT PRIMARY KEY,body TEXT,progression TEXT,background_id INTEGER, copyright_info TEXT);";
  std::cout << song_sql_command << "\n";
  rc = sqlite3_exec(*out_db, song_sql_command, callback, 0, &errmsg);
  if (rc != SQLITE_OK) {
    std::cerr << "SQL Error " << __LINE__ << errmsg << "\n";
    sqlite3_free(errmsg);
    return SQL_ERROR;
  }
  const char *sched_song_link_sql_cmd =
      "create table if not exists sched_song_links("
      "song_id INTEGER,"
      "sched_id INTEGER"
      ");";
  rc = sqlite3_exec(*out_db, sched_song_link_sql_cmd, callback, 0, &errmsg);
  if (rc != SQLITE_OK) {
    std::cerr << "SQL Error " << errmsg << "\n";
    sqlite3_free(errmsg);
    return SQL_ERROR;
  }
  return SUCCESS;
}
// opens the db and saves the song to it.
db_error saveSong(song input_song, char * db_name){
  
  // open db
  sqlite3 *db;
  int rc = sqlite3_open(db_name, &db);
	
  if (rc) {
    std::cerr << "Failed to open " << db_name << "\n";
    sqlite3_close(db);
    return SQL_ERROR;
  }
  //convert values to one sql query 
  std::stringstream ss;
  
  ss << "insert into songs VALUES('";
  
  // you have to use the c_str() function otherwise sqlite will complain that
  // there is an invalid token that you cannot see.
  // the caller of this function makes sure that there is a name
  //
  ss << input_song.name.c_str()<< "','"; 

  std::cerr << "size = " << input_song.body.size() << "\n"; 
  std::cerr << "body = " << input_song.body << "\n"; 
	ss << input_song.body.c_str() << "','";
  
  // TODO: add song bacground storage of some sort
  std::cerr << "size = " << input_song.progression.size() << "\n"; 
	ss << input_song.progression.c_str();

  ss << "',NULL,'";
  
	ss << input_song.copyright_info.c_str();
  ss <<"') on conflict (title) do update set "
"body=excluded.body,"
"progression=excluded.progression,"
"background_id=excluded.background_id,"
"copyright_info=excluded.copyright_info;";

  std::string sql = ss.str();
  std::cerr << "sql insert = " << sql << "\n";
  char * errmsg = NULL;
  
  rc = sqlite3_exec(db, sql.c_str(), callback, 0 ,&errmsg);
  std::cerr << sql << "\n";
  if (rc != SQLITE_OK){
	std::cerr << "SQL Error in " <<__FILE__ << ":" << __LINE__ << "\n" <<  errmsg << "\n";
	sqlite3_free(errmsg);
  sqlite3_close(db);
	return SQL_ERROR;
	}
  sqlite3_close(db);
  return SUCCESS;
}
  
// store all the data into all_songs
static int storeCallback(void * data, int argc, char **argv, char **colname){
  std::vector<song> *data_vec = (std::vector<song>*)data;
  
  // there should be 6 items:
  // title, body, progression, background_id, copyright_info, rowid
  // order in db:
  // rowid
  // - title
  // -body
  // - progression
  // - background_id
  // copyright_info
  
  // we can change the order in the query if necessary
  song tmp;
  tmp.id =  atoi(argv[0]);
  tmp.background_id = atoi(argv[4]);
  tmp.name = argv[1];
  tmp.body = argv[2];
  tmp.progression = argv[3];
  tmp.copyright_info = argv[5];
  
  data_vec->push_back(tmp);

  return 0;
}

// this assumes the db is open
db_error readSongs(sqlite3 * db, std::vector<song>& all_songs, std::vector<char*>& song_names){

  const char * query = "select rowid,* from songs;";
	char * err;
  if (sqlite3_exec(db, query, storeCallback, (void*)&all_songs, &err) != SQLITE_OK){
	std::cerr << "SQL Error at " << __LINE__ <<  ": " << err <<  "\n";
	sqlite3_free(err);
	return SQL_ERROR;

  }

  song_names.resize(all_songs.size());

  // populate song names
  // FIXME: this is pretty dangerous, we should probably allocate 
  // each char * and then copy, but the all_songs should not need to reallocate unless there is a save to the db
  // in  that case, we can just reassociate the two items.
  int end = all_songs.size();
  for(int i = 0; i <end; ++i){
	song_names[i] = (char*)all_songs[i].name.data();
  }

  return SUCCESS;
}
