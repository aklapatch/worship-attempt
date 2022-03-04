#pragma once
#include "structs.h"
void printglError(int lineNum){
  std::string output = "";
  switch(glGetError()){
    case GL_NO_ERROR: output = "NO ERROR"; break;
    case GL_INVALID_ENUM: output = "invalid enum"; break;
    case GL_INVALID_VALUE: output = "invalid value"; break;
    case GL_INVALID_OPERATION: output = "invalid operation"; break;
    case GL_INVALID_FRAMEBUFFER_OPERATION: output = "invalid framebuffer op"; break;
    case GL_OUT_OF_MEMORY: output = "out of memory"; break;
    case GL_STACK_UNDERFLOW: output = "stack underflow"; break;
    case GL_STACK_OVERFLOW: output = "stack overflow"; break;
    default: output = "no recognized error";
  }
  std::cerr << "GLError = " << output << " @ " << lineNum << "\n";
}

bool songEditor(std::vector<image> img_list, int &pic_num, int& font_size) {
  // song editor windows
  ImGui::InputText("Song title", (char*)titlebuf.c_str(), titlebuflen);
  ImGui::InputText("Song order", (char*)orderbuf.c_str(), orderbuflen);
  ImGui::InputTextMultiline("Song Lyrics", (char*)lyricbuf.c_str(), lyricbuflen);
  ImGui::InputText("Copyright Info", (char*)copyrightbuf.c_str(), copyrightbuflen); 
  int font_sz = font_size;
  ImGui::InputInt("font size", &font_sz);
  font_size = font_sz;
  static int curr_pic = 0;
      if (img_list.size() > 0){
  static char * curr_item = (char*)(img_list[0].name.data());
        if (ImGui::BeginCombo("pics", curr_item, 0)){
          int end = img_list.size();
          for (int i = 0; i < end; ++i){
            bool is_selected = (curr_item == img_list[i].name.data());
            if (ImGui::Selectable(img_list[i].name.c_str(), is_selected)){
              curr_item = (char*)(img_list[i].name.data());
              curr_pic = i;
            }
            if (is_selected){
              ImGui::SetItemDefaultFocus();
            }
          }
          ImGui::EndCombo();
        }

      }
      else {
        ImGui::Text("No Images found in database!");
      }


  bool reload_songs = false;
  bool no_title = false;
  // TODO: this object is pretty slow and probably not necessary
  static std::vector<std::string> errors = {};
  
  if (ImGui::Button("New Song")) {
    
    // clear all the buffers so that you can  start making a new song
    titlebuf.clear();
    orderbuf.clear();
    copyrightbuf.clear();
    lyricbuf.clear();
  }
ImGui::SameLine();
      db_error save_err = INIT_VAL;
  if (ImGui::Button("Save")) {
    // check if the fields are blank first
    // only the copyright field can be blank

    song song_to_save;
    song_to_save.name = titlebuf;
    song_to_save.body = lyricbuf; 
    song_to_save.progression = orderbuf;
    song_to_save.copyright_info = copyrightbuf;
    std::cerr <<  "title = " << titlebuf << "\n";
    
    // TODO: this does not work anymore (string is pre-allocated, 
    if( strlen(song_to_save.name.c_str()) == 0){
      no_title = true;
    }
    else{
      std::cerr << "saving\n";
      save_err = saveSong(song_to_save, db_name);
      reload_songs = true;
    }
    // TODO: print save error and a popup box
    //
    //
  }

    if (no_title){
      ImGui::OpenPopup("saveerr");
    }
    if (ImGui::BeginPopup("saveerr")){
      ImGui::Text("Please Insert a title");
      ImGui::EndPopup();
    }

  return reload_songs;
}

static void drawTextOnImage(Cairo::RefPtr<Cairo::Context> cr, std::string text, double font_size) {

  ///we will be altering data in tmp_text, so we will need a copy  of it
  std::string tmp_text(text);
  double x, y;
  cr->set_font_size(font_size);
  cr->get_current_point(x, y);
  // got through and move cr down when you hit a \n
  int i = 0;
  int start = 0;
  uintptr_t len = text.size();
  while (i < len) {

    if (tmp_text[i] == '\n') {
      // print the text from start to i
      tmp_text[i] = '\0';
      char *text_ptr = &(tmp_text[start]);
      cr->show_text(text_ptr);
      double x_new, y_new;
      cr->get_current_point(x_new, y_new);
      cr->move_to(x, y_new + font_size);
      start = i + 1;
    }
    i += 1;
  }

  if (start != i) {
    char *text_ptr = &(text[start]);
    cr->show_text(text_ptr);
  }
}

void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

GLFWwindow *full_win = NULL;
GLFWvidmode* mode = NULL; 

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void showMainMenuBar() {

  // main menu bar
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("schedule")) {
      ImGui::MenuItem("Save");
      ImGui::MenuItem("Open");
      ImGui::MenuItem("New");
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Songs")) {
      ImGui::MenuItem("Edit");
      ImGui::MenuItem("New");
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Output Display")) {
      // pressing this button should open a new menu to select the display to
      // output to
      int monitor_num = 0;
      GLFWmonitor **monitors = glfwGetMonitors(&monitor_num);
      int i = 0;
      if (monitor_num < 2) {
        ImGui::Text("Less than 2 monitors connected, cannot project to a "
                    "second display");
    }
    while (i < monitor_num) {

      std::string monitor_name = glfwGetMonitorName(monitors[i]);
      mode = ( GLFWvidmode *)glfwGetVideoMode(monitors[i]); 
      monitor_name.append(": ");

      monitor_name.append(std::to_string(mode->width));
      monitor_name.append("x");
      monitor_name.append(std::to_string(mode->height));

      if (ImGui::Button(monitor_name.c_str())){
        // destroy the old window if it exists
        if (full_win != NULL){
          glfwDestroyWindow(full_win);
        }


        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+

        full_win = glfwCreateWindow(mode->width, mode->height,"Full window", monitors[i], NULL);
        glfwSwapInterval(1); // Enable vsync
        // set the callback to exit when ESC is hit
        glfwSetKeyCallback(full_win, key_callback);
         
      }
    ++i;

      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }

  if(ImGui::IsKeyReleased(256) && full_win != NULL){
    // close the full screen window is ESC is pressed
    glfwDestroyWindow(full_win);
    full_win = NULL;
  }
}

// grab the title, then associate all the unnamed buffers that come after it
// (all the buffers taht do not have any [] in them.
std::vector<song_part> sepTitleParts(std::vector<std::string> text){
  
  std::vector<song_part> output;
  song_part tmp;
  tmp.name = "";
  uintptr_t text_len = text.size();
  uintptr_t title_start, title_end, search_pos, end_pos;
  for (uintptr_t i = 0; i < text_len; ++i){
    // if you find a [, then init a name. if a name is not provided, then just make it all one part 
    title_start = text[i].find('[', 0); 
    if(title_start != std::string::npos){
      // init name
      title_end = text[i].find(']', title_start);
      if (title_end == std::string::npos){
        std::cerr << "\ntitle parsing error, didn't find a ]\n";
      }
      else {
        // if there is a new title, then push the old paragraphcs
        if (tmp.paras.size() > 0){
            output.push_back(tmp);
        }
        // clear out/reset tmp
        tmp.name.clear();
        tmp.paras.clear();

        tmp.name = text[i].substr(title_start, title_end+1 - title_start);

        // push in everything after the title
        title_end++;
        if (text[i][title_end] == '\n'){
          title_end++;
        }
        tmp.paras.push_back(text[i].substr(title_end, text[i].size() - title_end));
      }
    }
    else{
      tmp.paras.push_back(text[i]);
    }

  }
  // if tmp was not named and has somthing in it, then the output should too. 
  // right now that loop above only pushes back if a name is detected
  if (tmp.paras.size() > 0){
    output.push_back(tmp);
  }

  return output;
}

bool isSpace(std::string input){
  uint32_t size = 0;
  for(auto x: input){
    if (x == ' ' || x == '\t' || x == '\n' || x == '\r'){
      ++size;
    }
  }
  return size == input.size();
}

std::vector<std::string> sepPara(std::string song_text){
  // separate paragraphs (two \n 's together)
  std::vector<std::string> output;
  uintptr_t end = song_text.size();
  uintptr_t start = 0;
  uintptr_t end_para = 0;
  while(start < end){
    end_para = song_text.find("\n\n", start);
    if (end_para == std::string::npos){
      end_para = end;
    }
    
    output.push_back(song_text.substr(start, end_para - start));
    start = end_para;
    if (song_text[start] == '\n'){
      ++start;
    }
  }
  return output;
}

// this algorithm should find brackets [ ] and
// assign the text between the brackets to the name field
// and assign what follows that (until you get to two \n in a row)
// if you don't find [], then you can make unnamed parts with empty strings
// for names 
std::vector<song_part> sepSongParts(std::string song_text) {

  // pull the \n\n separations out
  std::vector<std::string> paras = sepPara(song_text);
  
  // second phase, turn the bufs into song_parts 
  // assign blank names to the ones without [names]
  return sepTitleParts(paras);
}

db_error reloadSongs(std::vector<song> & all_songs, std::vector<char*>&song_names){


  all_songs.clear();
  all_songs.shrink_to_fit();
  song_names.clear();
  song_names.shrink_to_fit();
  std::cerr << "reloading songs" << "\n";
  sqlite3 *tmp_db;
  sqlite3_open(db_name, &tmp_db);
  readSongs(tmp_db, all_songs, song_names);
  sqlite3_close(tmp_db);

  return SUCCESS;
}

uint32_t scheduleBuilder(std::vector<song> &all_songs , std::vector<song>& sched_songs, std::vector<char*>& song_names){

  
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
    // need a songlist/order
    // need to implement a draggable menu for that
    //
    // this code is pulled from the imgui example code here:
    // https://github.com/ocornut/imgui/blob/master/imgui_demo.cpp
    // sched_song_list
    //
    // FIXME, this does not work when sched_song_list has any items in item

    ImGui::InputText("Song Title", (char*)title_buf.c_str(),title_buf_len);
    ImGui::InputText("Song Lyrics", (char*)lyric_buf.c_str(), lyric_buf_len);

    // when the user searches, the results will have to be populated into a
    // special string type that holds the results. then the result will pull the
    // song name out of a hash table or somthing, and edit that song.
    ImGui::Separator();
    ImGui::Text(
                "Double click a song in the list to add it, \nand hover over a song "
                "and hit the delete key to remove a song from the schedule.");

    static int32_t selected = -1;
    ImGui::Separator();
    if(ImGui::Button("Delete selected song from database.")){

      if (selected > -1){
        db_error err = deleteSong(db_name, all_songs[selected].name);
        if (err == SUCCESS){
          all_songs.erase(all_songs.begin() + (size_t)selected);
        }
        else{
          std::cerr << "Deleting song from database failed!\n";
        }
        reloadSongs(all_songs, song_names);

      }
    }
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
    
    ImGui::BeginChild("songch",ImVec2(ImGui::GetWindowContentRegionWidth()*0.5f, 0), true, window_flags );
    ImGui::Text("All Songs");

    ImGui::Separator();
    int size = all_songs.size();
    if (size == 0){
      ImGui::Text("No songs were found in the database.");
    }
    for(int i = 0; i < all_songs.size(); ++i){
      // only draw members that match the title.
      if(all_songs[i].name.find(title_buf.c_str()) <= 1){

        if(ImGui::Selectable(all_songs[i].name.c_str(), i == selected)){
          // load buffers for editor
          titlebuf = all_songs[i].name;
          titlebuf.reserve(titlebuflen);
          copyrightbuf = all_songs[i].copyright_info;
          copyrightbuf.reserve(copyrightbuflen);
          lyricbuf = all_songs[i].body;
          lyricbuf.reserve(lyricbuflen);
          orderbuf = all_songs[i].progression;
          orderbuf.reserve(orderbuflen);
          selected = i;

        }
        if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemActive()
            && ImGui::IsItemHovered()){
            sched_songs.push_back(all_songs[i]);
        }
      }
    }
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("sched_ch", ImVec2(0,300), true, window_flags);
    ImGui::Text("Schedule_songs");
    ImGui::Separator();
    int end = sched_songs.size();
    static int32_t selected_num = -1;
    for (int n = 0; n < end; ++n) {
      song item = sched_songs[n];

      if(ImGui::Selectable(sched_songs[n].name.c_str(), selected_num == n)){
        selected_num = n;
      }

      if (ImGui::IsItemActive() && !ImGui::IsItemHovered()) {
        selected_num = -1;
        int n_next = n + (ImGui::GetMouseDragDelta(0).y < 0.f ? -1 : 1);
        if (n_next >= 0 && n_next < (int)sched_songs.size()) {
          sched_songs[n] = sched_songs[n_next];
          sched_songs[n_next] = item;
          ImGui::ResetMouseDragDelta();
        }
      }
      if (ImGui::IsKeyReleased(261) && ImGui::IsItemHovered()) {

        // if a user presses delete when they are hovering over a song, delete
        // it.
        sched_songs.erase(sched_songs.begin() + n);
      }
    }
    ImGui::EndChild();
    return selected_num;
}


void schedSongUnit(
    std::vector<song> &all_songs ,
    std::vector<song>& sched_songs,
    std::vector<char*>& song_names,
    std::string & song_word_buf,
    std::vector<image> img_list,
    int &font_size,
    int &curr_pic
    ){

  ImGui::BeginChild("ChildL", ImVec2(ImGui::GetWindowContentRegionWidth()*0.5f,400), true);
  bool reload_songs = songEditor(img_list, curr_pic, font_size); 
  ImGui::EndChild();
  
  if(reload_songs){
    reloadSongs(all_songs, song_names);
  }

  ImGui::SameLine();
  ImGui::BeginChild("ChildR", ImVec2(0,400), true);
  int32_t selected_song_num = scheduleBuilder(all_songs , sched_songs,song_names);
  static int32_t prev_song_num;
  if (prev_song_num != selected_song_num && selected_song_num >= 0){
    // print out the separated song parts
    std::cerr << "Song's contents = \n";
    std::vector<song_part> parts = sepSongParts(sched_songs[selected_song_num].body);
    for (auto x: parts){
      std::cerr << "title = " << x.name << "\n";
      for (auto y: x.paras){
        std::cerr << "para = " << y << "\n";
      }
    }

  }
  prev_song_num = selected_song_num;
  ImGui::EndChild();

  ImGui::Text("Rendered_song");
  // separate the song's parts and draw them as different boxes to click on
  // TODO: optimize this so that it does not recopy the song every time.
  if ( selected_song_num >= 0 && sched_songs.size() > 0){
    song curr_song = sched_songs[selected_song_num];

    std::vector<song_part> parts = sepSongParts(curr_song.body);
    // print buttons for text 
  // TODO: get stuff and do stuff with it.
  // separate song parts into verses that are labeled with teh proper sytnax
  // lay them all out as buttons with the proper labes (from the titles taht they were given
  // I still need to associate the verses and stuff with progression.
    uintptr_t end = parts.size();
    
    for (uintptr_t i = 0; i < end; ++i){
      if (parts[i].name.size() > 0){
        ImGui::Text(parts[i].name.c_str());
      }
      for (auto x: parts[i].paras){
      if(ImGui::Button(x.c_str())){
        // copy string contents to display on image
        song_word_buf = x;
        }
      }
    }
  }
}
