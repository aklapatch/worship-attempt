

#pragma once
#include <cairomm/cairomm.h>

#include "structs.h"
#include "imgui/imgui.h"
#include "imgui/examples/imgui_impl_glfw.h"
#include "imgui/examples/imgui_impl_opengl3.h"
#include "sqlite_routines.h"
#include"fileMenu.h"

#include<GL/gl3w.h>

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
#include <opencv2/imgcodecs.hpp>

bool imageMenu(std::vector<image>&);
