
#pragma once
#include "imgui/imgui.h"
#include "imgui/examples/imgui_impl_glfw.h"
#include "imgui/examples/imgui_impl_opengl3.h"
#include<dirent.h>
#include<unistd.h>
#include<string>
#include <vector>
#include <cstdlib>
#include "sqlite_routines.h"

bool fileMenu(std::string &file_name);
