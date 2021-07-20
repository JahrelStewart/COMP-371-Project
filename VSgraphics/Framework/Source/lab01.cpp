// COMP 371 Assignment 1
// Created by Jahrel Stewart

#include <iostream>
#include<fstream>
#include<string>
#include <sstream>

#define GLEW_STATIC 1   // This allows linking with Static Library on Windows, without DLL
#include <GL/glew.h>    // Include GLEW - OpenGL Extension Wrangler

#include <GLFW/glfw3.h> // GLFW provides a cross-platform interface for creating a graphical context,
                        // initializing OpenGL and binding inputs

#include <glm/glm.hpp>  // GLM is an optimized math library with syntax to similar to OpenGL Shading Language
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

int gridSize = 100;
int totalTiles = (int)(pow(gridSize, 2.0) + 0.5);
int wallSize = 5;
int totalWallCubes = (int)(pow(wallSize, 2.0) + 0.5);
int wallCubeExclude = 0;
int amtCubeVertices = 0;

//Camera essential properties:
glm::vec3 cameraPos = glm::vec3(0.0f, 20.0f, 60.0f);
glm::vec3 cameraLookAt = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraLookAt);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraRightDirection = glm::cross(cameraUp, cameraDirection);

glm::mat4 scaleModelMatrix = glm::mat4(1.0f);
float growModel = 1.005f;
float shrinkModel = 0.995f;
glm::mat4 rotateModelMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
float rotateModel = 5.0f;
glm::mat4 moveModelMatrix = glm::mat4(1.0f);
float moveModel = 0.5f;
glm::mat4 worldOrientMatrix = glm::mat4(1.0f);
float worldOrient = 0.5f;
glm::vec2 camPanTiltZoom(-90.0f, 0.0f), preCamPanTiltZoom(-90.0f, 0.0f);
glm::vec2 camWorldOrient(0.0f, 0.0f), preCamWorldOrient(0.0f, 0.0f);
float zoom = 55.0f, preZoom = 55.0f;
bool checkPos = true;
glm::vec2 lastPos(0.0f, 0.0f);

float modelDimensions = 20.0f;

struct Shaders {
    std::string GridShaderSource;
    std::string ThreeLineShaderSource;
    std::string WallShaderSource;
    std::string CubeShaderSource;
    std::string FragmentShaderSource;
    std::string CubeFragmentShaderSource;
};

static Shaders readShaders(const std::string& file) {
    std::ifstream stream(file);

    enum class whichShader {
        EMPTY = -1, GRIDSHADER = 0, THREELINESHADER = 1, WALLSHADER = 2, CUBESHADER = 3, FRAGMENTSHADER = 4, CUBEFRAGMENTSHADER = 5
    };

    std::stringstream shaderStream[6];
    std::string line;
    whichShader which = whichShader::EMPTY;

    while (getline(stream, line)) {
        if (line.find("#shader") != std::string::npos) {
            if (line.find("gridShader") != std::string::npos) {
                which = whichShader::GRIDSHADER;
            }
            else if (line.find("threeLineShader") != std::string::npos) {
                which = whichShader::THREELINESHADER;
            }
            else if (line.find("wallShader") != std::string::npos) {
                which = whichShader::WALLSHADER;
            }
            else if (line.find("cubeShader") != std::string::npos) {
                which = whichShader::CUBESHADER;
            }
            else if (line.find("fragmentShader") != std::string::npos) {
                which = whichShader::FRAGMENTSHADER;
            }
            else if (line.find("cubeFragmentShader") != std::string::npos) {
                which = whichShader::CUBEFRAGMENTSHADER;
            }
        }
        else {
            shaderStream[(int)which] << line << '\n';
        }
    }

    return { shaderStream[0].str(), shaderStream[1].str(), shaderStream[2].str(), shaderStream[3].str(), shaderStream[4].str(), shaderStream[5].str() };
}

void mouseCalculations(GLFWwindow* window, double currentXpos, double currentYpos) {
    if (checkPos) {
        lastPos.x = currentXpos;
        lastPos.y = currentYpos;
        checkPos = false;
    }

    zoom = preZoom;
    camPanTiltZoom.x = preCamPanTiltZoom.x;
    camPanTiltZoom.y = preCamPanTiltZoom.y;
    camWorldOrient.x = preCamWorldOrient.x;
    camWorldOrient.y = preCamWorldOrient.y;

    float offsetX = lastPos.x - currentXpos;
    camPanTiltZoom.x += offsetX * 0.15f;
    camWorldOrient.x += offsetX * 0.15f;

    float offsetY = lastPos.y - currentYpos;
    zoom += offsetY * 0.1f;

    float camMoveOffsetY = currentYpos - lastPos.y;
    camPanTiltZoom.y += camMoveOffsetY * 0.15f;
    camWorldOrient.y += camMoveOffsetY * 0.15f;


    if (zoom >= 150.0f) {
        zoom = 150.0f;
    }
    if (zoom <= 2.0f) {
        zoom = 2.0f;
    }

    camPanTiltZoom.y = std::max(-85.0f, std::min(85.0f, camPanTiltZoom.y));
    camWorldOrient.y = std::max(-85.0f, std::min(85.0f, camWorldOrient.y));

    lastPos.x = currentXpos;
    lastPos.y = currentYpos;
}

int setupVertexAndFragmentShader(GLuint shaderType, const std::string& shaderSRC) {
    int shader = glCreateShader(shaderType);
    const char* shaderSource = shaderSRC.c_str();
    glShaderSource(shader, 1, &shaderSource, nullptr);
    glCompileShader(shader);

    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    return shader;
}

int compileAndLinkShaders(const std::string& vertexShaderSRC, const std::string& fragmentShaderSRC) {
    // TODO
    // compile and link shader program
    // return shader program id
    // ------------------------------------

    // vertex shader
    int vertexShader = setupVertexAndFragmentShader(GL_VERTEX_SHADER, vertexShaderSRC);
    // fragment shader
    int fragmentShader = setupVertexAndFragmentShader(GL_FRAGMENT_SHADER, fragmentShaderSRC);

    // link shaders
    int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    char infoLog[512];
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;

}

int createVertexArrayObject() {
    // TODO
    // Upload geometry to GPU and return the Vertex Buffer Object ID

    //ground surafce vertex points
    glm::vec3 gridVertexArray[] = {
        //positions                   colors
        glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3((float)170 / 255, (float)185 / 255, (float)207 / 255),
        glm::vec3(-0.5f, 0.5f, 0.0f), glm::vec3((float)170 / 255, (float)185 / 255, (float)207 / 255),
        glm::vec3(0.5f, 0.5f, 0.0f), glm::vec3((float)170 / 255, (float)185 / 255, (float)207 / 255),
        glm::vec3(0.5f, -0.5f, 0.0f), glm::vec3((float)170 / 255, (float)185 / 255, (float)207 / 255),
    };

    //creating indexes for index buffer to avoid dupilicate vertices
    unsigned int gridIndexes[] = {
        0, 1, 2,
        2, 3, 0
    };

    //We create the 100 x 100 grid here to avoid calling the draw-call many times. This will boost performance
    glm::vec3 quadPositons[10000];
    int gridIndex = 0;
    //offset centers all tiles around the origin for both X and Y planes....later will rotate the model 90 degrees so that it flat on the XZ plane
    float gridOffset = float((gridSize / 2.0f) - 0.5f);
    //Draw each tile row by row on the XY plane
    for (int i = 0; i < gridSize; i++) {
        for (int j = 0; j < gridSize; j++) {
            glm::vec3 quadPos;
            quadPos.x = (float)(j - gridOffset);
            quadPos.y = (float)(i - gridOffset);
            quadPos.z = 0.0f;
            quadPositons[gridIndex++] = quadPos;
        }
    }

    //Vertices for set of three lines
    glm::vec3 threeLinesVertexArray[] = {
        //positions                  color
        glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), //x axis line start-point
        glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), //x axis line end-point 
        glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), //y axis line start-point 
        glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), //y axis line end-point 
        glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f),  //z axis line start-point 
        glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 1.0f)  //z axis line end-point 
    };

    //Vertices for Wall
    glm::vec3 wallVertexArray[] = {
        //positions                     
        glm::vec3(-2.0f, -2.0f, 1.0f), //0
        glm::vec3(-2.0f, 2.0f, 1.0f), //1
        glm::vec3(2.0f, 2.0f, 1.0f),  //2
        glm::vec3(2.0f, -2.0f, 1.0f),  //3
        glm::vec3(-2.0f, -2.0f, -1.0f), //4
        glm::vec3(-2.0f, 2.0f, -1.0f), //5
        glm::vec3(2.0f, -2.0f, -1.0f), //6
        glm::vec3(2.0f, 2.0f, -1.0f), //7
    };

    //We create the 5x5 grid here to avoid calling the draw-call many times. This will boost performance
    glm::vec3 wallPositons[25];
    int wallIndex = 0;
    //offset centers all cubes around the origin for both X and Y planes
    float wallOffset = float(((wallSize * 4) / 2.0f) - 2.0f);
    //Draw each tile row by row on the XY plane
    for (int i = 0; i < (4 * wallSize); i = i + 4) {
        for (int j = 0; j < (4 * wallSize); j = j + 4) {
            if (((i >= 4 && i <= 8) && (j >= 4 && j <= 12)) || (i == 12 && j == 8)) {
                wallCubeExclude++;
                continue;
            }

            glm::vec3 wallPos;
            wallPos.x = (float)(j - wallOffset);
            wallPos.y = (float)(i - wallOffset) + 20.0f;
            wallPos.z = 0.0f;
            wallPositons[wallIndex++] = wallPos;
        }
    }

    glm::vec3 cubesVertexArray[] = {
        //positions                     
        glm::vec3(-2.0f, 18.0f, 14.0f), //0
        glm::vec3(-2.0f, 22.0f, 14.0f), //1
        glm::vec3(2.0f, 22.0f, 14.0f),  //2
        glm::vec3(2.0f, 18.0f, 14.0f),  //3
        glm::vec3(-2.0f, 18.0f, 10.0f), //4
        glm::vec3(-2.0f, 22.0f, 10.0f), //5
        glm::vec3(2.0f, 18.0f, 10.0f), //6
        glm::vec3(2.0f, 22.0f, 10.0f), //7
    };

    //creating indexes for index buffer to avoid dupilicate vertices when creating a cube
    unsigned int cubeIndexes[] = {
        0, 1, 2, //Front Face
        2, 3, 0,

        4, 5, 1, //Left Face
        1, 0, 4,

        6, 7, 5, //Back Face
        5, 4, 6,

        3, 2, 7, //Right Face
        7, 6, 3,

        3, 6, 4, // Bottom Face
        4, 0, 3,

        1, 5, 7, // Top Face
        7, 2, 1
    };

    amtCubeVertices = sizeof(cubeIndexes) / sizeof(*cubeIndexes);

    // Create a vertex array    
    GLuint vertexArrayObject;
    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);

    // Upload Vertex Buffer to the GPU, keep a reference to it (vertexBufferObject)
    GLuint gridVertexBufferObject;
    glGenBuffers(1, &gridVertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, gridVertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gridVertexArray), gridVertexArray, GL_STATIC_DRAW);

    glVertexAttribPointer(0,                    // attribute 0 matches a Pos in Vertex Shader
        3,                                      // size
        GL_FLOAT,                               // type
        GL_FALSE,                               // normalized?
        2 * sizeof(glm::vec3),                  // stride - each vertex contain 2 vec3 (position, color)
        (void*)0                               // array buffer offset
    );
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1,            // attribute 1 matches a Color in Vertex Shader
        3,
        GL_FLOAT,
        GL_FALSE,
        2 * sizeof(glm::vec3),
        (void*)sizeof(glm::vec3)      // color is offseted a vec3 (comes after position)
    );
    glEnableVertexAttribArray(1);

    //grid instances
    GLuint gridInstancesBufferObject;
    glGenBuffers(1, &gridInstancesBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, gridInstancesBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * totalTiles, &quadPositons[0], GL_STATIC_DRAW);

    glVertexAttribPointer(2,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(glm::vec3),
        (void*)0
    );
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);


    //Grid Index Buffer
    GLuint gridIndexBufferObject;
    glGenBuffers(1, &gridIndexBufferObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gridIndexBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), gridIndexes, GL_STATIC_DRAW);


    //vertex object array for three axis lines
    GLuint threeLinesBufferObject;
    glGenBuffers(1, &threeLinesBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, threeLinesBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(threeLinesVertexArray), threeLinesVertexArray, GL_STATIC_DRAW);

    glVertexAttribPointer(3,
        3,                      // size
        GL_FLOAT,               // type
        GL_FALSE,               // normalized?
        2 * sizeof(glm::vec3),  // stride - each vertex contain 3 vec3 (position, color)
        (void*)0               // array buffer offset
    );
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(4,
        3,
        GL_FLOAT,
        GL_FALSE,
        2 * sizeof(glm::vec3),
        (void*)sizeof(glm::vec3)
    );
    glEnableVertexAttribArray(4);

    // vertex object array for Wall
    GLuint wallBufferObject;
    glGenBuffers(1, &wallBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, wallBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(wallVertexArray), wallVertexArray, GL_STATIC_DRAW);

    glVertexAttribPointer(5,                    // attribute 0 matches a Pos in Vertex Shader
        3,                                      // size
        GL_FLOAT,                               // type
        GL_FALSE,                               // normalized?
        sizeof(glm::vec3),                      // stride - each vertex contain 2 vec3 (position, color)
        (void*)0                               // array buffer offset
    );
    glEnableVertexAttribArray(5);


    //wall cube instances
    GLuint wallInstancesBufferObject;
    glGenBuffers(1, &wallInstancesBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, wallInstancesBufferObject);
    glBufferData(GL_ARRAY_BUFFER, (totalWallCubes - wallCubeExclude) * sizeof(glm::vec3), &wallPositons[0], GL_STATIC_DRAW);

    glVertexAttribPointer(6,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(glm::vec3),
        (void*)0
    );
    glEnableVertexAttribArray(6);
    glVertexAttribDivisor(6, 1);

    //Wall Index Buffer
    GLuint wallIndexBufferObject;
    glGenBuffers(1, &wallIndexBufferObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wallIndexBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, amtCubeVertices * sizeof(unsigned int), cubeIndexes, GL_STATIC_DRAW);

    // vertex object array for Wall
    GLuint cubesBufferObject;
    glGenBuffers(1, &cubesBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, cubesBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubesVertexArray), cubesVertexArray, GL_STATIC_DRAW);

    glVertexAttribPointer(7,                   // attribute 0 matches a Pos in Vertex Shader
        3,                   // size
        GL_FLOAT,            // type
        GL_FALSE,            // normalized?
        sizeof(glm::vec3), // stride - each vertex contain 2 vec3 (position, color)
        (void*)0             // array buffer offset
    );
    glEnableVertexAttribArray(7);


    //Cube Index Buffer
    GLuint cubesIndexBufferObject;
    glGenBuffers(1, &cubesIndexBufferObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubesIndexBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, amtCubeVertices * sizeof(unsigned int), cubeIndexes, GL_STATIC_DRAW);

    glBindVertexArray(0);

    return vertexArrayObject;
}


int main(int argc, char* argv[]) {
    // Initialize GLFW and OpenGL version
    glfwInit();

    // Create Window and rendering context using GLFW, resolution is 800x600
    GLFWwindow* window = glfwCreateWindow(1024, 768, "Comp371 - Assignment 1", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to create GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }

    Shaders shader = readShaders("shaders.shader");

    // Smokey Dark Blue background
    glClearColor((float)13 / 255, (float)19 / 255, (float)33 / 255, 1.0f);

    // Define and upload geometry to the GPU here ...
    int vao = createVertexArrayObject();

    //calculate view matrix..which is essentially the view of the camera        
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraLookAt, cameraUp);

    //create projection matrix
    glm::mat4 projection = glm::perspective(glm::radians(zoom), (float)1024 / (float)768, 0.1f, 100.0f);

    glm::mat4 cubePos[] = { glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 4.0f)),
                            glm::translate(glm::mat4(1.0f), glm::vec3(-4.0f, 0.0f, 4.0f)),
                            glm::translate(glm::mat4(1.0f), glm::vec3(-4.0f, -4.0f, 4.0f)),
                            glm::translate(glm::mat4(1.0f), glm::vec3(4.0f, 0.0f, 4.0f)),
                            glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 4.0f, 4.0f)),
                            glm::translate(glm::mat4(1.0f), glm::vec3(4.0f, -4.0f, 4.0f)),
                            glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -4.0f)),
                            glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 4.0f, -4.0f)),
                            glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -4.0f, -4.0f)),
                            glm::translate(glm::mat4(1.0f), glm::vec3(-4.0f, -4.0f, -4.0f)),
                            glm::translate(glm::mat4(1.0f), glm::vec3(4.0f, -4.0f, -4.0f))
    };


    glfwSetCursorPosCallback(window, mouseCalculations);
    glEnable(GL_DEPTH_TEST);
    int shaderProgram = 0;
    // std::cout << glGetString(GL_VERSION);
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);    

    // Entering Main Loop
    while (!glfwWindowShouldClose(window)) {
        // Each frame, reset color of each pixel to glClearColor
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(vao);

        //Draw the grid
        {
            //calculate model matrix..which is the transform of the Grid
            glm::mat4 model = worldOrientMatrix * glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); //Rotate grid 90 degrees so that it lays flat on the XZ plane
            int shaderProgram = compileAndLinkShaders(shader.GridShaderSource, shader.FragmentShaderSource);
            glUseProgram(shaderProgram);
            int model_location = glGetUniformLocation(shaderProgram, "u_Model_View_Projection");
            glm::mat4 transformation_result = projection * view * model;
            glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(transformation_result));

            glLineWidth(1.0f);
            glDrawElementsInstanced(GL_LINE_LOOP, 5, GL_UNSIGNED_INT, nullptr, totalTiles);
        }

        //Draw three-colored axis lines
        {
            glm::mat4 model = worldOrientMatrix * glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            int shaderProgram = compileAndLinkShaders(shader.ThreeLineShaderSource, shader.FragmentShaderSource);
            glUseProgram(shaderProgram);
            int model_location = glGetUniformLocation(shaderProgram, "u_Model_View_Projection");
            glm::mat4 transformation_result = projection * view * model;
            glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(transformation_result));

            glLineWidth(5.0f);
            glDrawArrays(GL_LINES, 0, 6);
        }

        //Draw Wall
        {
            // Translate to origin, then scale and then translate back to original position. This will cause the scaling to happen from the origin of the object
            glm::mat4 model = worldOrientMatrix;
            model *= moveModelMatrix;
            model *= rotateModelMatrix;
            model *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -modelDimensions + 20.0f, 0.0f)) * scaleModelMatrix * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));

            int shaderProgram = compileAndLinkShaders(shader.WallShaderSource, shader.CubeFragmentShaderSource);
            glUseProgram(shaderProgram);
            int model_location_MVP = glGetUniformLocation(shaderProgram, "u_Model_View_Projection");
            int model_location_Color = glGetUniformLocation(shaderProgram, "u_Color");

            glm::mat4 transformation_result = projection * view * model;
            glm::vec3 color = glm::vec3((float)102 / 255, (float)215 / 255, (float)209 / 255);
            glUniformMatrix4fv(model_location_MVP, 1, GL_FALSE, glm::value_ptr(transformation_result));
            glUniform3f(model_location_Color, color.x, color.y, color.z);

            glLineWidth(1.0f);
            glDrawElementsInstanced(GL_TRIANGLES, amtCubeVertices, GL_UNSIGNED_INT, nullptr, totalWallCubes - wallCubeExclude);
        }

        //Draw Cubes
        glm::mat4 parentCube;

        for (int i = 0; i < 12; i++) {
            glm::mat4 model;
            if (i == 0) {
                model = worldOrientMatrix;

                model *= moveModelMatrix;
                model *= rotateModelMatrix;
                model *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -modelDimensions + 20.0f, 0.0f)) * scaleModelMatrix * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
                parentCube = model;
            }
            else {
                model = parentCube;
                model *= cubePos[i - 1];
            }

            int shaderProgram = compileAndLinkShaders(shader.CubeShaderSource, shader.CubeFragmentShaderSource);
            glUseProgram(shaderProgram);
            int model_location_MVP = glGetUniformLocation(shaderProgram, "u_Model_View_Projection");
            int model_location_Color = glGetUniformLocation(shaderProgram, "u_Color");

            glm::mat4 transformation_result = projection * view * model;

            glUniformMatrix4fv(model_location_MVP, 1, GL_FALSE, glm::value_ptr(transformation_result));

            glLineWidth(2.0f);

            //Draw Cube Lines (Optional...for aesthetic purposes):
            // glm::vec3 color = glm::vec3((float) 128 / 255, (float) 128 / 255, (float) 128 / 255);
            // glUniform3f(model_location_Color, color.x, color.y, color.z);
            // glDrawElements(GL_LINE_LOOP, 30, GL_UNSIGNED_INT, nullptr);

            glLineWidth(1.0f);
            //Color Cube Faces
            glm::vec3 color = glm::vec3((float)233 / 255, (float)230 / 255, (float)255 / 255);
            glUniform3f(model_location_Color, color.x, color.y, color.z);
            glDrawElements(GL_TRIANGLES, amtCubeVertices, GL_UNSIGNED_INT, nullptr);

        }

        glBindVertexArray(0);

        // End frame
        glfwSwapBuffers(window);

        // Detect inputs
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // Model Scale, Position, Rotate
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            moveModelMatrix = glm::translate(moveModelMatrix, glm::vec3(-moveModel, 0.0f, 0.0f));
        }

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            moveModelMatrix = glm::translate(moveModelMatrix, glm::vec3(moveModel, 0.0f, 0.0f));
        }

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            moveModelMatrix = glm::translate(moveModelMatrix, glm::vec3(0.0f, 0.0f, -moveModel));
        }

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            moveModelMatrix = glm::translate(moveModelMatrix, glm::vec3(0.0f, 0.0f, moveModel));
        }

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            rotateModelMatrix = glm::rotate(rotateModelMatrix, glm::radians(-rotateModel), glm::vec3(0.0f, 1.0f, 0.0f));
        }

        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            rotateModelMatrix = glm::rotate(rotateModelMatrix, glm::radians(rotateModel), glm::vec3(0.0f, 1.0f, 0.0f));
        }

        if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
            scaleModelMatrix = glm::scale(scaleModelMatrix, glm::vec3(growModel, growModel, growModel));
            modelDimensions *= growModel;
        }

        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
            scaleModelMatrix = glm::scale(scaleModelMatrix, glm::vec3(shrinkModel, shrinkModel, shrinkModel));
            modelDimensions *= shrinkModel;
        }

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            modelDimensions = 20.0f;
            scaleModelMatrix = glm::mat4(1.0f);
            rotateModelMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            moveModelMatrix = glm::mat4(1.0f);
        }

        //World Orientation        
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            cameraPos.x -= worldOrient;
            view = glm::lookAt(cameraPos, cameraPos + cameraLookAt, cameraUp);
        }

        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            cameraPos.x += worldOrient;
            view = glm::lookAt(cameraPos, cameraPos + cameraLookAt, cameraUp);
        }

        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            cameraPos.z -= worldOrient;
            view = glm::lookAt(cameraPos, cameraPos + cameraLookAt, cameraUp);
        }

        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            cameraPos.z += worldOrient;
            view = glm::lookAt(cameraPos, cameraPos + cameraLookAt, cameraUp);
        }

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            worldOrientMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(camWorldOrient.y), glm::vec3(1.0f, 0.0f, 0.0f));
            worldOrientMatrix = glm::rotate(worldOrientMatrix, glm::radians(-camWorldOrient.x), glm::vec3(0.0f, 1.0f, 0.0f));

            preCamWorldOrient.x = camWorldOrient.x;
            preCamWorldOrient.y = camWorldOrient.y;
        }

        if (glfwGetKey(window, GLFW_KEY_HOME) == GLFW_PRESS) {
            worldOrientMatrix = glm::mat4(1.0f);
            cameraLookAt = glm::vec3(0.0f, 0.0f, -1.0f);
            cameraPos = glm::vec3(0.0f, 20.0f, 60.0f);
            view = glm::lookAt(cameraPos, cameraPos + cameraLookAt, cameraUp);

            zoom = 55.0f;
            preZoom = 55.0f;
            projection = glm::perspective(glm::radians(zoom), (float)1024 / (float)768, 0.1f, 100.0f);

            camPanTiltZoom = glm::vec2(-90.0f, 0.0f);
            preCamPanTiltZoom = glm::vec2(-90.0f, 0.0f);
            camWorldOrient = glm::vec2(0.0f, 0.0f);
            preCamWorldOrient = glm::vec2(0.0f, 0.0f);
        }

        //Render Modes:
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        }

        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }

        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        //Camera Pan,Tilt, Zoom:
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_R) != GLFW_PRESS) {
            projection = glm::perspective(glm::radians(zoom), (float)1024 / (float)768, 0.1f, 100.0f);
            preZoom = zoom;
        }

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_R) != GLFW_PRESS) {
            cameraLookAt.x = cos(glm::radians(camPanTiltZoom.x)) * cos(glm::radians(preCamPanTiltZoom.y));
            cameraLookAt.z = sin(glm::radians(camPanTiltZoom.x)) * cos(glm::radians(preCamPanTiltZoom.y));
            view = glm::lookAt(cameraPos, cameraPos + glm::normalize(glm::vec3(cameraLookAt.x, cameraLookAt.y, cameraLookAt.z)), cameraUp);
            preCamPanTiltZoom.x = camPanTiltZoom.x;
        }

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_R) != GLFW_PRESS) {
            cameraLookAt.y = sin(glm::radians(camPanTiltZoom.y));
            cameraLookAt.z = sin(glm::radians(preCamPanTiltZoom.x)) * cos(glm::radians(camPanTiltZoom.y));
            view = glm::lookAt(cameraPos, cameraPos + glm::normalize(glm::vec3(cameraLookAt.x, cameraLookAt.y, cameraLookAt.z)), cameraUp);
            preCamPanTiltZoom.y = camPanTiltZoom.y;
        }

    }

    glDeleteProgram(shaderProgram);
    // Shutdown GLFW
    glfwTerminate();

    return 0;
}
