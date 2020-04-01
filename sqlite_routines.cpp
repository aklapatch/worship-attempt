#include<iostream>
#include<stdlib.h>
#include<sqlite3.h>


static int callback(void *notUsed, int argc, char ** argv, char ** colname) {
    for(int i = 0; i < argc; ++i){
      std::cerr << colname[i] << " = " << (argv[i] ? argv[i] : "NULL") << "\n";
    }
    std::cerr << "\n";
    return 0;
}

// opens out_db and leaves it open for the program
int init_and_open_db(char * fname, sqlite3 **out_db) {
    int rc = sqlite3_open(fname, out_db); 
    if (rc){
        std::cerr << "Failed to open " << fname << "\n";
        sqlite3_close(*out_db);
        return 1;
    }
    char * errmsg = NULL;
    
    const char * picture_sql_cmd = "create table if not exists main.pictures(id integer primary key asc,name text,data blob);";

    rc = sqlite3_exec(*out_db,picture_sql_cmd, callback, 0, &errmsg);
    if (rc != SQLITE_OK){
      std::cerr << "SQL Error " << __LINE__ << " "<< errmsg << "\n";
        sqlite3_free(errmsg);
	return 2;
    }
    const char * sched_sql_cmd = "create table if not exists main.schedules("
            "id INTEGER PRIMARY KEY ASC,"
            "name TEXT"
            ");";

    rc = sqlite3_exec(*out_db,sched_sql_cmd, callback, 0, &errmsg);
    if (rc != SQLITE_OK){
      std::cerr << "SQL Error " << __LINE__<< errmsg << "\n";
        sqlite3_free(errmsg);
	return 2;
    }

    const char * song_sql_command = "create table if not exists main.songs("
            "id INTEGER PRIMARY KEY ASC,"
            "title TEXT,"
            "body TEXT,"
      "progression TEXT,"
            "background_id INTEGER, "
            "FOREIGN KEY(background_id) REFERENCES pictures(id)"
            ");";
    std::cout << song_sql_command << "\n";
    rc = sqlite3_exec(*out_db,song_sql_command, callback, 0, &errmsg);
    if (rc != SQLITE_OK){
      std::cerr << "SQL Error " << __LINE__ << errmsg << "\n";
        sqlite3_free(errmsg);
	return 2;
    }
    const char * sched_song_link_sql_cmd = "create table if not exists mainsched_song_links("
            "song_id INTEGER,"
            "sched_id INTEGER,"
            "FOREIGN KEY(song_id) REFERENCES songs(id),"
            "FOREIGN KEY(sched_id) REFERENCES schedules(id)"
            ");";
    rc = sqlite3_exec(*out_db,sched_song_link_sql_cmd, callback, 0, &errmsg);
    if (rc != SQLITE_OK){
      std::cerr << "SQL Error " << errmsg << "\n";
        sqlite3_free(errmsg);
	return 2;
    }
    return 0;
}
