#pragma once
#include<string>
#include<fstream>
#include<vector>
#include<sstream>
#include<string>
#include <GL/gl3w.h> // Initialize with gl3wInit()

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path);
