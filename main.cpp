
#include <cairomm/cairomm.h>
#include "structs.h"
#include "imgui/imgui.h"
#include "imgui/examples/imgui_impl_glfw.h"
#include "imgui/examples/imgui_impl_opengl3.h"
#include "sqlite_routines.h"
#include"imageMenu.h."
#include"fileMenu.h"
#include "gl_util.h"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdio.h>

// leave this here after all the other headers
#include"imGLsetup.h"

#include"shader_class.h"

const char *vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"layout (location = 2) in vec2 aTexCoord;\n"
"out vec3 ourColor;\n"
"out vec2 TexCoord;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos, 1.0);\n"
"ourColor = aColor;\n"
"TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"
"}\0";
const char *fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 ourColor;\n"
"in vec2 TexCoord;\n"
"uniform sampler2D texture1;\n"
"void main()\n"
"{\n"
"   FragColor = texture(texture1, TexCoord);\n"
"}\n\0";

// buffers for song editor
const int copyrightbuflen = 100;
std::string copyrightbuf(copyrightbuflen, '\0' );

const int lyricbuflen = 1024;
std::string lyricbuf(lyricbuflen, '\0' );

const int titlebuflen = 64;
std::string titlebuf(titlebuflen, '\0');

const int orderbuflen = 64;
std::string orderbuf(orderbuflen, '\0');

std::string img_list;

char db_name[] = "outtest.db";

const uint32_t title_buf_len = 200;
std::string title_buf(title_buf_len+1, '\0');

const uint32_t lyric_buf_len = 200;
std::string lyric_buf(lyric_buf_len+1, '\0');

std::vector<char> pic_names(0);

#include"misc-util.h"

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
  const char *glsl_version = "#version 330";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+
#endif

  // Create window with graphics context
  GLFWwindow *window = glfwCreateWindow(
      1280, 720, "Worship Attempt", NULL, NULL);
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

  ImGui::StyleColorsDark();

  // Setup Platform/Renderer bindings
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  ImGuiIO &io = ImGui::GetIO();
  ImFont* font1 = io.Fonts->AddFontFromFileTTF("ubuntu-font-family/Ubuntu-C.ttf", 16.0f);

  cv::Mat orig_image;

  cv::Mat cairo_img;
  cv::Mat tmp_image;
  cv::Mat disp_img;
  cv::Mat out_img;

  Cairo::RefPtr<Cairo::ImageSurface> surf;
Cairo::RefPtr<Cairo::Context> cr;
int from_to[] = {0, 3, 1, 2, 2, 1, 3, 0};
  Cairo::RefPtr<Cairo::ToyFontFace> font = Cairo::ToyFontFace::create(
      "Candara", Cairo::FONT_SLANT_ITALIC, Cairo::FONT_WEIGHT_BOLD);

  // try executing sqlite commands
  sqlite3 *main_db;
  init_and_open_db(db_name, &main_db);
  std::vector<image> img_list;
  readImgs(img_list);

  std::vector<song> all_song_list, sched_song_list;
  std::vector<char*> song_names;
  readSongs(main_db, all_song_list, song_names);
  sqlite3_close(main_db);
  //===========================================
  // Our state
  bool show_demo_window = true;
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  GLuint shaderProgram;
  GLuint win_tex;
  GLuint img_tex;

  static uint32_t word_buf_size = 512;
  static std::string song_word_buf(word_buf_size, '\0');

  static std::string tmp_word_buf(word_buf_size, '\0');
  static int font_size = 10;

  GLfloat vertices[] = {
    // positions          // colors           // texture coords
    1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
    1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
    -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
    -1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
  };
  unsigned int indices[] = {  
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
  };
  GLuint VBO, VAO, EBO;

  // Main loop
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    showMainMenuBar();

    // the font gets corrupted sometimes if you remove these two lines
    ImGui::PushFont(font1);
    ImGui::PopFont();

    ImGui::Begin("Main Window");
    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    static int curr_pic = -1;
    if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags)){
      // I tried to make this return an int32_t and then the window failed to draw

      if (ImGui::BeginTabItem("Schedule Stuff")){
        schedSongUnit(all_song_list,sched_song_list, song_names, song_word_buf , img_list, font_size, curr_pic);
        ImGui::EndTabItem();
      }

      static bool reload_imgs = true;

      if (ImGui::BeginTabItem("Image Menu")){
        if(reload_imgs){

          readImgs(img_list);
        }
        reload_imgs = imageMenu(img_list);
        ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
    }

    ImGui::End();


    if (show_demo_window)
      ImGui::ShowDemoWindow(&show_demo_window);

    {
      ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!"
      // and append into it.

      ImGui::Checkbox(
          "Demo Window",
          &show_demo_window); // Edit bools storing our window open/close state

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
          1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
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

    static bool first_time = true;
    // take care of the fullscreen window last
    if (full_win != NULL && glfwWindowShouldClose(full_win)){
      glfwDestroyWindow(full_win);
      first_time = true;
      full_win = NULL;
    }
    if (full_win != NULL && !glfwWindowShouldClose(full_win)){

      glfwMakeContextCurrent(full_win);
      if (first_time){
        glfwSwapInterval(1); // Enable vsync
        first_time = false;

        // init shader
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);
        // check for shader compile errors
        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (success == GL_FALSE)
        {
          glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
          std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        // fragment shader
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);
        // check for shader compile errors
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (success == GL_FALSE)
        {
          glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
          std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        // link shaders
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        // check for linking errors
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
          glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
          std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
        glDetachShader(shaderProgram, vertexShader);
        glDetachShader(shaderProgram, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // color attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        // texture coord attribute
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        int h = mode->height;
        int w = mode->width;
        out_img = cv::Mat(h,w, disp_img.type());
        std::cerr << "dims = " << out_img.size() << "\n";

        glGenTextures(1, &win_tex);

      }
        static int curr_pic = -1;
        static int old_curr_pic = -2;
        if (img_list.size() > 0){
          static char * curr_item = (char*)img_list[0].name.data();
          curr_pic = 0;

          if (curr_pic != old_curr_pic){
            // reload orig image into cairo
            orig_image = img_list[curr_pic].datamat;
            // convert to argb for cairo
            cv::cvtColor(orig_image, orig_image, cv::COLOR_BGR2BGRA);
            cairo_img = cv::Mat(orig_image.size(), orig_image.type());
            tmp_image = cv::Mat(orig_image.size(), orig_image.type());
            disp_img= cv::Mat(orig_image.size(), orig_image.type());

            // convert BGRA -> ARGB
  int stride = Cairo::ImageSurface::format_stride_for_width(
      Cairo::FORMAT_ARGB32, orig_image.cols);
            cv::mixChannels(&orig_image, 1, &cairo_img, 1, from_to, 4);
            surf =
              Cairo::ImageSurface::create(cairo_img.data, Cairo::FORMAT_ARGB32,
                  orig_image.cols, orig_image.rows, stride);

          }
          old_curr_pic = curr_pic;

          if (curr_pic > -1) {
            // only do the update/copy if the font size/text has changed.
            static int old_font_size = 0;
            static int old_text_size = -1;
            if (old_font_size != font_size || old_text_size != song_word_buf.size()){
  int stride = Cairo::ImageSurface::format_stride_for_width(
      Cairo::FORMAT_ARGB32, orig_image.cols);
            cv::mixChannels(&orig_image, 1, &cairo_img, 1, from_to, 4);
            surf =
              Cairo::ImageSurface::create(cairo_img.data, Cairo::FORMAT_ARGB32,
                  orig_image.cols, orig_image.rows, stride);
              cr = Cairo::Context::create(surf);
              cr->move_to(50, 100);
              cr->set_font_face(font);
              cr->set_font_size(font_size);

              // write text on image and do all the things you need to do to show it.
              drawTextOnImage(cr, song_word_buf, font_size);

              std::copy(surf->get_data(),
                  surf->get_data() + (orig_image.rows * orig_image.cols * 4),
                  tmp_image.data);

              int final_from_to[] = {0, 3, 1, 0, 2, 1, 3, 2};
              cv::mixChannels(&tmp_image, 1, &disp_img, 1, final_from_to, 4);

              //  reload texture with image data
              glGenTextures(1, &img_tex);
              glBindTexture(GL_TEXTURE_2D, img_tex);
              glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
              glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

              glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
              glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, disp_img.cols, disp_img.rows, 0,
                  GL_RGBA, GL_UNSIGNED_BYTE, disp_img.data);

      cv::resize(disp_img, out_img, out_img.size(), 0,0, cv::INTER_CUBIC);
      cv::Mat final_img(out_img.size(), out_img.type());
      cv::rotate(out_img, final_img, cv::ROTATE_180);
      cv::flip(final_img, out_img, 1);
      glBindTexture(GL_TEXTURE_2D, win_tex);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
      // this image does not update at all, even if the image data changes
      glTexImage2D(GL_TEXTURE_2D, 
          0, 
          GL_RGBA8, 
          mode->width, 
          mode->height, 
          0, 
          GL_RGBA, 
          GL_UNSIGNED_BYTE, 
          out_img.data);
            }
            old_font_size = font_size;
            old_text_size = song_word_buf.size();

          }
        }


      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);


      glEnable(GL_TEXTURE_2D);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, win_tex);
      glUseProgram(shaderProgram);
      glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

      glBindVertexArray(VAO);
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

      glDisable(GL_TEXTURE_2D);

      glfwSwapBuffers(full_win);
    }

    glDeleteTextures(1, &img_tex);

    glfwMakeContextCurrent(window);
  }
  glDeleteTextures(1, &win_tex);

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
