
// dear imgui: standalone example application for GLFW + OpenGL 3, using
// programmable pipeline If you are new to dear imgui, see examples/README.txt
// and documentation at the top of imgui.cpp. (GLFW is a cross-platform general
// purpose library for handling windows, inputs, OpenGL/Vulkan graphics context
// creation, etc.)

#include "structs.h"
#include <cairomm/cairomm.h>

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

// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load
//  OpenGL function pointers. Helper libraries are often used for this purpose!
//  Here we are supporting a few common ones (gl3w, glew, glad). You may use
//  another loader/header of your choice (glext, glLoadGen, etc.), or chose to
//  manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h> // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h> // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h> // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING)
#define GLFW_INCLUDE_NONE // GLFW including OpenGL headers causes ambiguity or
                          // multiple definition errors.
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h> // Initialize with glbinding::initialize()
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to
// maximize ease of testing and compatibility with old VS compilers. To link
// with VS2010-era libraries, VS2015+ requires linking with
// legacy_stdio_definitions.lib, which we do using this pragma. Your own project
// should not be affected, as you are likely to link with a newer binary of GLFW
// that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) &&                                 \
    !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void drawTextOnImage(Cairo::RefPtr<Cairo::Context> cr, char *text,
                            int len, double font_size) {

  char *tmp_text = new char[len];
  std::copy(text, text + len, tmp_text);

  double x, y;
  cr->set_font_size(font_size);
  cr->get_current_point(x, y);
  // got through and move cr down when you hit a \n
  int i = 0;
  int start = 0;
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
  delete[] tmp_text;
}

static void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

static void showMainMenuBar() {

  // main menu bar
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("Schedule")) {
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
      } else {
        while (i < monitor_num) {

          const char *monitor_name = glfwGetMonitorName(monitors[i]);
          ImGui::Text("%s", monitor_name);
          ++i;
        }
      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
}

    // this algorithm should find brackets [ ] and
    // assign the text between the brackets to the name field
    // and assign what follows that (until you get to two \n in a row)
    // `
std::vector<song_part> sepSongParts(char *f_input, uintptr_t len) {

  std::vector<song_part> output = {};
  std::vector<char> tmp(f_input, f_input+len);
  tmp[len] = '\0';
  char * input = tmp.data();
  uintptr_t input_len = len;
  std::cerr << input_len << "\n";

  char * input_end = input + input_len -1;
  // break once you hit the end
  while (true) {
    // a bracketed name should come before text otherwise
    // FIXME: this will miss text before the first title
    // FIXME: this will fail if there is a [ at EOF
    char *part_title_start= strchr(input, '[') ;
    std::cerr << part_title_start << "\n";
    if (part_title_start == NULL){
      return output;
    }

    // FIXME: this will bug out if ] is at the beginning
    char *part_title_end = strchr(part_title_start, ']') ;
    std::cerr << part_title_end << "\n";
    if (part_title_end == NULL){
      return output;
    }
    uintptr_t diff =  part_title_end - part_title_start;

    std::vector<char> name(diff, '_');
    std::copy(part_title_start, part_title_end, name.data());
    
    // find the \n\n to separate parts
    input = part_title_end;
    char *part_start = part_title_end;
    char * part_end = strchr(part_start, '[');
    if (part_end == NULL){
      part_end = input_end; 
    }
    diff = part_end - part_start;

    std::vector<char> text(diff, ' ');
    std::copy(part_start,part_end, text.data());
    
    // TODO: find a way to preallocate this
    output.push_back({name, text});
    
    // index input
    input = part_end;
  }
  
  return output;
}

// buffers for song editor
const int lyricbuflen = 1024;
static char lyricbuf[lyricbuflen + 1] = {};

const int titlebuflen = 64;
static char titlebuf[titlebuflen ] = {};

const int orderbuflen = 64;
static char orderbuf[orderbuflen + 1] = {};

static bool show_nothing_msg = false;

static void songEditor() {
  // song editor windows
  ImGui::Begin("Song Editor");
  ImGui::InputText("Song title", titlebuf, titlebuflen);
  ImGui::InputText("Song order", orderbuf, orderbuflen);
  ImGui::InputTextMultiline("Song Lyrics", lyricbuf, lyricbuflen);
  static int item_type = 1;
  ImGui::Combo("Background Select", &item_type, "default\0other_image\0", 2);
  if (ImGui::Button("Parse")) {
    show_nothing_msg = true;
    std::cerr << "229\n";
    std::vector<song_part> song_parts = sepSongParts(lyricbuf, lyricbuflen); 
    std::cerr << "here\n";
    printf("%d\n", song_parts.size() );
    for (auto x: song_parts){

      x.name.push_back('\0');
      x.text.push_back('\0');
    char * name  = x.name.data();
    char * text  = x.text.data();
    std::cerr << "title = " << name << "\n";
    std::cerr << "text = " << text << "\n";
    }
    }


 
  ImGui::End();
}

int main(int, char **) {
    orderbuf[orderbuflen] = '\0';
    titlebuf[titlebuflen ] = '\0';
    lyricbuf[lyricbuflen ] = '\0';
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
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable
  // Keyboard Controls io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; //
  // Enable Gamepad Controls

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
  int sq_err = init_and_open_db("outtest.db", &main_db);
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

  const int title_searchbuf_len = 64;
  char title_searchbuf[title_searchbuf_len + 1] = {};

  const int lyric_searchbuf_len = 64;
  char lyric_searchbuf[title_searchbuf_len + 1] = {};


  const char *list[] = {"song1\0", "song2\0", "song3\0"};

  std::vector<const char *> sched_song_list = {}, all_song_list = {};
  all_song_list.push_back(list[0]);
  all_song_list.push_back(list[1]);
  all_song_list.push_back(list[2]);
  sched_song_list.push_back(list[0]);

  const int word_buf_size = 200;
  static char song_word_buf[word_buf_size];

  static char tmp_word_buf[word_buf_size];

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
    /*
       ImGui::Begin("Song selector");
       ImGui::InputText("Song Title", title_searchbuf, title_searchbuf_len);
       ImGui::InputText("Song Lyrics", lyric_searchbuf, lyric_searchbuf_len);

    // when the user searches, the results will have to be populated into a
    special string type that holds the results.
    // then the result will pull the song name out of a hash table or somthing,
    and edit that song. static int song_num = 1; static bool ret = false;

    const char *items[] = {"song", "song1", "song2"};
    ImGui::ListBox("Song List",&song_num,items, IM_ARRAYSIZE(items),
    IM_ARRAYSIZE(items) );

    if (ImGui::Button("Search (does not work)")){
    }



    ImGui::End();
    */

    ImGui::Begin("Schedule Builder");
    // need a songlist/order
    // need to implement a draggable menu for that
    //
    // this code is pulled from the imgui example code here:
    // https://github.com/ocornut/imgui/blob/master/imgui_demo.cpp
    // sched_song_list
    //
    // FIXME, this does not work when sched_song_list has any items in item
    // the selectable function segfaults
    for (int n = 0; n < sched_song_list.size(); n++) {
      const char *item = sched_song_list[n];
      ImGui::Selectable(sched_song_list[n]);

      if (ImGui::IsItemActive() && !ImGui::IsItemHovered()) {
        int n_next = n + (ImGui::GetMouseDragDelta(0).y < 0.f ? -1 : 1);
        if (n_next >= 0 && n_next < sched_song_list.size()) {
          sched_song_list[n] = sched_song_list[n_next];
          sched_song_list[n_next] = item;
          ImGui::ResetMouseDragDelta();
        }
      }
      if (ImGui::IsKeyReleased(261) && ImGui::IsItemHovered()) {

        // if a user presses delete when they are hovering over a song, delete
        // it.
        sched_song_list.erase(sched_song_list.begin() + n);
      }
    }

    /*
       if (ImGui::Button("add song")){

       }
       if (ImGui::Button("remove song")){

       }
       */
    ImGui::Separator();
    ImGui::InputText("Song Title", title_searchbuf, title_searchbuf_len);
    ImGui::InputText("Song Lyrics", lyric_searchbuf, lyric_searchbuf_len);

    // when the user searches, the results will have to be populated into a
    // special string type that holds the results. then the result will pull the
    // song name out of a hash table or somthing, and edit that song.
    static int song_num = 1;
    static int drag_num = 1;
    static bool ret = false;
    ImGui::ListBox("Song List", &song_num, all_song_list.data(),
                   all_song_list.size());
    for (int j = 0; j < IM_ARRAYSIZE(io.MouseDown); ++j) {

      if (ImGui::IsMouseDoubleClicked(j) && ImGui::IsItemHovered()) {
        sched_song_list.push_back(all_song_list[song_num]);
      }
    }

    ImGui::Text(
        "Double click a song in the list to add it, \nand hover over a song "
        "and hit the delete key to remove a song from the schedule.");

    // schedule name box(probably not necessary)
    //
    ImGui::End(); // sched builder

    songEditor();

    ImGui::Begin("img_test");
    ImGui::InputTextMultiline("word input", song_word_buf, word_buf_size);
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
    drawTextOnImage(cr, song_word_buf, word_buf_size, font_size);

    // TODO: during the copy, check if something is different, and if it is,
    // updated the texture.

    // go through the image and separate the \n bits into separate strings.
    static int i = 0;
    static int start = 0;
    static bool replace = false;
    static int num = 0;
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
