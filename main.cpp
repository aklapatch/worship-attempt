
// dear imgui: standalone example application for GLFW + OpenGL 3, using
// programmable pipeline If you are new to dear imgui, see examples/README.txt
// and documentation at the top of imgui.cpp. (GLFW is a cross-platform general
// purpose library for handling windows, inputs, OpenGL/Vulkan graphics context
// creation, etc.)
#include <cairomm/cairomm.h>

#include "structs.h"
#include "imgui/imgui.h"
#include "imgui/examples/imgui_impl_glfw.h"
#include "imgui/examples/imgui_impl_opengl3.h"
#include "sqlite_routines.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <stdio.h>
#include <string>

// leave this here after all the other headers
#include"imGLsetup.h"

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

static void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

static void showMainMenuBar() {

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

    const char *monitor_name = glfwGetMonitorName(monitors[i]);
    ImGui::Text("%s", monitor_name);
    ++i;

      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
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

// buffers for song editor
const int copyrightbuflen = 100;
static std::string copyrightbuf(copyrightbuflen, '\0' );

const int lyricbuflen = 1024;
static std::string lyricbuf(lyricbuflen, '\0' );

const int titlebuflen = 64;
static std::string titlebuf(titlebuflen, '\0');

const int orderbuflen = 64;
static std::string orderbuf(orderbuflen, '\0');

char db_name[] = "outtest.db";

static void errPopup(const char* msg){
}
static bool songEditor() {
  // song editor windows
  ImGui::InputText("Song title", (char*)titlebuf.c_str(), titlebuflen);
  ImGui::InputText("Song order", (char*)orderbuf.c_str(), orderbuflen);
  ImGui::InputTextMultiline("Song Lyrics", (char*)lyricbuf.c_str(), lyricbuflen);
  ImGui::InputText("Copyright Info", (char*)copyrightbuf.c_str(), copyrightbuflen); 
  static int item_type = 1;
  ImGui::Combo("Background Select", &item_type, "default\0other_image\0", 2);

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
      // TODO: if the compiler optimizes this out, this may not work
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

const uint32_t title_buf_len = 200;
static std::string title_buf(title_buf_len+1, '\0');

const uint32_t lyric_buf_len = 200;
static std::string lyric_buf(lyric_buf_len+1, '\0');

static auto vector_getter(void* vec, int idx, const char ** out_text)
{
  std::vector<std::string> *vec_ptr = (std::vector<std::string>*)vec;
  if (idx < 0 || idx >= vec_ptr->size()){return false;}
  *out_text = vec_ptr->at(idx).c_str();
  return true;
}

static uint32_t scheduleBuilder(std::vector<song> all_songs , std::vector<song>& sched_songs, std::vector<char*>& song_names){

  
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

    ImGui::Separator();
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
    
    ImGui::BeginChild("songch",ImVec2(ImGui::GetWindowContentRegionWidth()*0.5f, 0), true, window_flags );
    ImGui::Text("All Songs");

    ImGui::Separator();
    int size = all_songs.size();
    static uint32_t selected = 0;
    for(int i = 0; i < size; ++i){
      // only draw members that match the title.
      if(all_songs[i].name.find(title_buf.c_str()) <= 1){

        if(ImGui::Selectable(all_songs[i].name.c_str())){
          // load buffers for editor
          titlebuf = all_songs[i].name;
          titlebuf.reserve(titlebuflen);
          copyrightbuf = all_songs[i].copyright_info;
          copyrightbuf.reserve(copyrightbuflen);
          lyricbuf = all_songs[i].body;
          lyricbuf.reserve(lyricbuflen);
          orderbuf = all_songs[i].progression;
          orderbuf.reserve(orderbuflen);

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
        if (n_next >= 0 && n_next < sched_songs.size()) {
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

static void schedSongUnit(std::vector<song> &all_songs , std::vector<song>& sched_songs, std::vector<char*>& song_names ,std::string & song_word_buf){

  ImGui::Begin("Schedule");
  ImGui::BeginChild("ChildL", ImVec2(ImGui::GetWindowContentRegionWidth()*0.5f,400), true);
  bool reload_songs = songEditor(); 
  ImGui::EndChild();
  
  if(reload_songs){
    all_songs.clear();
    std::cerr << "reloading songs" << "\n";
    sqlite3 *tmp_db;
    sqlite3_open(db_name, &tmp_db);
    readSongs(tmp_db, all_songs, song_names);
    sqlite3_close(tmp_db);
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

  ImGui::End(); // Schedule
  
}

int main(int, char **) {
  // Setup window
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
    return 1;

    // Decide GL+GLSL versions
#if __APPLE__
  // GL 3.2 + GLSL 150
  const char *glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac
#else
  // GL 3.0 + GLSL 130
  const char *glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+
  // only glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // 3.0+ only
#endif

  // Create window with graphics context
  GLFWwindow *window = glfwCreateWindow(
      1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
  if (window == NULL)
    return 1;
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync

  // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
  bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
  bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
  bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING)
  bool err = false;
  glbinding::initialize([](const char *name) {
    return (glbinding::ProcAddress)glfwGetProcAddress(name);
  });
#else
  bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader
                    // is likely to requires some form of initialization.
#endif
  if (err) {
    fprintf(stderr, "Failed to initialize OpenGL loader!\n");
    return 1;
  }

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;

  // Setup Dear ImGui style

  ImGui::StyleColorsDark();
  // ImGui::StyleColorsClassic();

  // Setup Platform/Renderer bindings
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  cv::Mat orig_image = cv::imread("bell_v22.png", cv::IMREAD_COLOR);

  // convert to argb for cairo
  cv::cvtColor(orig_image, orig_image, cv::COLOR_BGR2BGRA);
  cv::Mat cairo_img(orig_image.size(), orig_image.type());
  cv::Mat tmp_image(orig_image.size(), orig_image.type());
  // convert BGRA -> ARGB
  int from_to[] = {0, 3, 1, 2, 2, 1, 3, 0};
  cv::mixChannels(&orig_image, 1, &cairo_img, 1, from_to, 4);
  // cv::cvtColor(image, image, cv::COLOR_BGRA2RGBA)
  int stride = Cairo::ImageSurface::format_stride_for_width(
      Cairo::FORMAT_ARGB32, orig_image.cols);
  Cairo::RefPtr<Cairo::ImageSurface> surf =
      Cairo::ImageSurface::create(cairo_img.data, Cairo::FORMAT_ARGB32,
                                  orig_image.cols, orig_image.rows, stride);
  Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(surf);
  Cairo::RefPtr<Cairo::ToyFontFace> font = Cairo::ToyFontFace::create(
      "Candara", Cairo::FONT_SLANT_ITALIC, Cairo::FONT_WEIGHT_BOLD);
  cr->move_to(100, 50);
  cr->set_font_face(font);

  std::copy(surf->get_data(),
            surf->get_data() + (orig_image.rows * orig_image.cols * 4),
            tmp_image.data);
  // now convert argb -> RGBA
  cv::Mat disp_img(tmp_image.size(), tmp_image.type());
  int final_from_to[] = {0, 3, 1, 0, 2, 1, 3, 2};
  cv::mixChannels(&tmp_image, 1, &disp_img, 1, final_from_to, 4);

  // set up opengl to get image texture
  GLuint img_tex;
  glGenTextures(1, &img_tex);
  glBindTexture(GL_TEXTURE_2D, img_tex);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tmp_image.cols, tmp_image.rows, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, disp_img.data);

  // try executing sqlite commands
  sqlite3 *main_db;
  init_and_open_db(db_name, &main_db);
  
  std::vector<song> all_song_list, sched_song_list;
  std::vector<char*> song_names;
  readSongs(main_db, all_song_list, song_names);
  sqlite3_close(main_db);

  // Load Fonts
  // - If no fonts are loaded, dear imgui will use the default font. You can
  // also load multiple fonts and use ImGui::PushFont()/PopFont() to select
  // them.
  // - AddFontFromFileTTF() will return the ImFont* so you can store it if you
  // need to select the font among multiple.
  // - If the file cannot be loaded, the function will return NULL. Please
  // handle those errors in your application (e.g. use an assertion, or display
  // an error and quit).
  // - The fonts will be rasterized at a given size (w/ oversampling) and stored
  // into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which
  // ImGui_ImplXXXX_NewFrame below will call.
  // - Read 'docs/FONTS.txt' for more instructions and details.
  // - Remember that in C/C++ if you want to include a backslash \ in a string
  // literal you need to write a double backslash \\ !
  // io.Fonts->AddFontDefault();
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
  // ImFont* font =
  // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f,
  // NULL, io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != NULL);

  // Our state
  bool show_demo_window = true;
  bool show_another_window = false;
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  static uint32_t word_buf_size = 512;
  static std::string song_word_buf(word_buf_size+1, '\0');

  static std::string tmp_word_buf(word_buf_size+1, '\0');

  // Main loop
  while (!glfwWindowShouldClose(window)) {
    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to
    // tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to
    // your main application.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input
    // data to your main application. Generally you may always pass all inputs
    // to dear imgui, and hide them from your application based on those two
    // flags.
    glfwPollEvents();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    showMainMenuBar();
    ImGuiIO &io = ImGui::GetIO();

    // I tried to make this return an int32_t and then the window failed to draw
     schedSongUnit(all_song_list,sched_song_list, song_names, song_word_buf );

    ImGui::Begin("img_test");
    static int font_size = 10;
    ImGui::InputInt("font size", &font_size);
    // write text on image and do all the things you need to do to show it.
    //
    // reload orig image into cairo
    // TODO: only reload image when text or font size changes
    cv::mixChannels(&orig_image, 1, &cairo_img, 1, from_to, 4);
    surf =
        Cairo::ImageSurface::create(cairo_img.data, Cairo::FORMAT_ARGB32,
                                    orig_image.cols, orig_image.rows, stride);
    cr = Cairo::Context::create(surf);
    cr->move_to(50, 100);
    cr->set_font_face(font);
    cr->set_font_size(font_size);
    drawTextOnImage(cr, song_word_buf, font_size);

    // TODO: during the copy, check if something is different, and if it is,
    // updated the texture.

    // go through the image and separate the \n bits into separate strings.
    std::copy(surf->get_data(),
              surf->get_data() + (orig_image.rows * orig_image.cols * 4),
              tmp_image.data);

    int final_from_to[] = {0, 3, 1, 0, 2, 1, 3, 2};
    cv::mixChannels(&tmp_image, 1, &disp_img, 1, final_from_to, 4);

    //
    //  reload texture with image data
    //  TODO: only reload texture when setting or text changes.
    glBindTexture(GL_TEXTURE_2D, img_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tmp_image.cols, tmp_image.rows, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, disp_img.data);

    ImGui::Image((void *)(intptr_t)img_tex,
                 ImVec2(tmp_image.cols, tmp_image.rows));
    ImGui::End();

    // 1. Show the big demo window (Most of the sample code is in
    // ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear
    // ImGui!).
    if (show_demo_window)
      ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair
    // to created a named window.
    {
      static float f = 0.0f;
      static int counter = 0;

      ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!"
                                     // and append into it.

      ImGui::Text("This is some useful text."); // Display some text (you can
                                                // use a format strings too)
      ImGui::Checkbox(
          "Demo Window",
          &show_demo_window); // Edit bools storing our window open/close state
      ImGui::Checkbox("Another Window", &show_another_window);

      ImGui::SliderFloat("float", &f, 0.0f,
                         1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
      ImGui::ColorEdit3(
          "clear color",
          (float *)&clear_color); // Edit 3 floats representing a color

      if (ImGui::Button("Button")) // Buttons return true when clicked (most
                                   // widgets return true when edited/activated)
        counter++;
      ImGui::SameLine();
      ImGui::Text("counter = %d", counter);

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                  1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
      ImGui::End();
    }

    // 3. Show another simple window.
    if (show_another_window) {
      ImGui::Begin(
          "Another Window",
          &show_another_window); // Pass a pointer to our bool variable (the
                                 // window will have a closing button that will
                                 // clear the bool when clicked)
      ImGui::Text("Hello from another window!");
      if (ImGui::Button("Close Me"))
        show_another_window = false;
      ImGui::End();
    }

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
