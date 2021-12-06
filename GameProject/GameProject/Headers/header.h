#pragma once
#include <iostream>
#include <vector>
#include<fstream>
#include<string>
#include <sstream>
#include <filesystem>
#include <map>

#define GLEW_STATIC 1   // This allows linking with Static Library on Windows, without DLL
#include <GL/glew.h>    // Include GLEW - OpenGL Extension Wrangler

#include <GLFW/glfw3.h> // GLFW provides a cross-platform interface for creating a graphical context,
						// initializing OpenGL and binding inputs

#include <glm/glm.hpp>  // GLM is an optimized math library with syntax to similar to OpenGL Shading Language
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H 

 #include <irrKlang/irrKlang.h>
 using namespace irrklang;

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../Resources/stb_image/stb_image.h"

int compileAndLinkShaders(const std::string& vertexShaderSRC, const std::string& fragmentShaderSRC, const std::string* geometryShaderSRC = nullptr);