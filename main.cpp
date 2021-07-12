//
// COMP 371 Assignment Framework
//
// Created by Jahrel Stewart
//
// Inspired by the following tutorials:
// - https://learnopengl.com/Getting-started/Hello-Window
// - https://learnopengl.co m/Getting-started/Hello-Triangle

#include <iostream>


#define GLEW_STATIC 1   // This allows linking with Static Library on Windows, without DLL
#include <GL/glew.h>    // Include GLEW - OpenGL Extension Wrangler

#include <GLFW/glfw3.h> // GLFW provides a cross-platform interface for creating a graphical context,
                        // initializing OpenGL and binding inputs

#include <glm/glm.hpp>  // GLM is an optimized math library with syntax to similar to OpenGL Shading Language
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

int grid_size = 100;
int totalTiles = (int) (pow(grid_size, 2.0) + 0.5);

const char* getThreeLinesShaderSource() {
    // TODO - Insert Vertex Shaders here  ...
    // For now, you use a string for your shader code, in the assignment, shaders will be stored in .glsl files
    const char* vs = R"glsl(
        #version 330 core
        layout (location = 3) in vec3 aLinePos;
        layout (location = 4) in vec3 aLineColor;
        out vec3 vertexColor;

        uniform mat4 u_Model_View_Projection;        

        void main()
        {
            vertexColor = aLineColor;
            gl_Position = u_Model_View_Projection * vec4(aLinePos, 1.0);
        }
    )glsl";

    return vs;
}

const char* getVertexShaderSource() {
    // TODO - Insert Vertex Shaders here  ...
    // For now, you use a string for your shader code, in the assignment, shaders will be stored in .glsl files
    const char* vs = R"glsl(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aColor;
        layout (location = 2) in vec3 offsetPos;
        out vec3 vertexColor;

        uniform mat4 u_Model_View_Projection;        

        void main()
        {
            vertexColor = aColor;
            gl_Position = u_Model_View_Projection * vec4(aPos + offsetPos, 1.0);
        }
    )glsl";

    return vs;
}


const char* getFragmentShaderSource() {
    // TODO - Insert Fragment Shaders here ...
    const char* fs = R"glsl(
        #version 330 core
        in vec3 vertexColor;
        out vec4 FragColor;
        void main()
        {
            FragColor = vec4(vertexColor.r, vertexColor.g, vertexColor.b, 1.0f);
        }
    )glsl";

    return fs;
}

int setupVertexAndFragmentShader(GLuint shaderType, const char& shaderSRC) {
    int shader = glCreateShader(shaderType);
    const char* shaderSource = &shaderSRC;
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

int compileAndLinkShaders(const char& vertexShaderSRC, const char& fragmentShaderSRC) {
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
    glm::vec3 vertexArray[] = {
        //positions                   colors
        glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3((float) 170 / 255, (float) 185 / 255, (float) 207 / 255),
        glm::vec3(-0.5f, 0.5f, 0.0f), glm::vec3((float) 170 / 255, (float) 185 / 255, (float) 207 / 255),
        glm::vec3(0.5f, 0.5f, 0.0f), glm::vec3((float) 170 / 255, (float) 185 / 255, (float) 207 / 255),
        glm::vec3(0.5f, -0.5f, 0.0f), glm::vec3((float) 170 / 255, (float) 185 / 255, (float) 207 / 255),
    };

    //creating indexes for index buffer to avoid dupilicate vertices
    unsigned int indexes[] = {
        0, 1, 2,
        2, 3, 0
    };

    //We create the 100 x 100 grid here to avoid calling the draw-call many times. This will boost performance
    glm::vec3 quadPositons[10000];
    int index = 0;
    //offset centers all tiles around the origin for both X and Z planes
    float offset = float((grid_size / 2.0f) - 0.5f);
    //Draw each tile row by row on the XZ plane
    for (int i = 0; i < grid_size; i++) {
        for (int j = 0; j < grid_size; j++) {
            glm::vec3 quadPos;
            quadPos.x = (float) (j - offset);
            quadPos.y = (float) (i - offset);
            quadPos.z = 0.0f;
            quadPositons[index++] = quadPos;
        }
    }

    //Vertices for set of three lines
    glm::vec3 threeLinesVertexArray[] = {
        //pos                color
        glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), //x axis line start-point
        glm::vec3(0.5f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), //x axis line end-point 
        glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), //y axis line start-point 
        glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), //y axis line end-point 
        glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f),  //z axis line start-point 
        glm::vec3(0.0f, 0.0f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f)  //z axis line end-point 
    };

    // Create a vertex array
    GLuint vertexArrayObject;
    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);

    // Upload Vertex Buffer to the GPU, keep a reference to it (vertexBufferObject)
    GLuint vertexBufferObject;
    glGenBuffers(1, &vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexArray), vertexArray, GL_STATIC_DRAW);

    glVertexAttribPointer(0,                   // attribute 0 matches a Pos in Vertex Shader
        3,                   // size
        GL_FLOAT,            // type
        GL_FALSE,            // normalized?
        2 * sizeof(glm::vec3), // stride - each vertex contain 2 vec3 (position, color)
        (void*) 0             // array buffer offset
    );
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1,            // attribute 1 matches a Color in Vertex Shader
        3,
        GL_FLOAT,
        GL_FALSE,
        2 * sizeof(glm::vec3),
        (void*) sizeof(glm::vec3)      // color is offseted a vec3 (comes after position)
    );
    glEnableVertexAttribArray(1);

    //Index Buffer
    GLuint indexBufferObject;
    glGenBuffers(1, &indexBufferObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indexes, GL_STATIC_DRAW);

    //object instances
    GLuint instancesBufferObject;
    glGenBuffers(1, &instancesBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, instancesBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * totalTiles, &quadPositons[0], GL_STATIC_DRAW);

    glVertexAttribPointer(2,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(glm::vec3),
        (void*) 0
    );
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);

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
        (void*) 0               // array buffer offset
    );
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(4,
        3,
        GL_FLOAT,
        GL_FALSE,
        2 * sizeof(glm::vec3),
        (void*) sizeof(glm::vec3)
    );
    glEnableVertexAttribArray(4);

    glBindVertexArray(0); // Unbind to not modify the VAO

    return vertexArrayObject;
}


int main(int argc, char* argv[]) {
    // Initialize GLFW and OpenGL version
    glfwInit();

#if defined(PLATFORM_OSX)	
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    // On windows, we set OpenGL version to 2.1, to support more hardware
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#endif

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

    // Smokey Dark Blue background
    glClearColor((float) 13 / 255, (float) 19 / 255, (float) 33 / 255, 1.0f);

    // Compile and link shaders here ...
    int shaderProgram = compileAndLinkShaders(*getVertexShaderSource(), *getFragmentShaderSource());

    // Define and upload geometry to the GPU here ...
    int vao = createVertexArrayObject();

    //calculate model matrix..which is the transform of the object we want to draw 
    glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); //Rotate grid 90 degrees so that it lays flat on the XZ plane

    //calculate view matrix..which is essentially the view of the camera         
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, -80.0f));
    view = glm::rotate(view, glm::radians(20.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    //create projection matrix...which is like mapping the coordinates of the current space to -1 to 1 space
    glm::mat4 projection = glm::perspective(glm::radians(55.0f), (float) 1024 / (float) 768, 0.1f, 100.0f);

    glUseProgram(shaderProgram);
    int model_location = glGetUniformLocation(shaderProgram, "u_Model_View_Projection");
    glm::mat4 transformation_result = projection * view * model;
    glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(transformation_result));

    // std::cout << glGetString(GL_VERSION);

    // Entering Main Loop
    while (!glfwWindowShouldClose(window)) {
        // Each frame, reset color of each pixel to glClearColor
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(vao);
        glDrawArraysInstanced(GL_LINE_LOOP, 0, 4, totalTiles);
        // glDrawArrays(GL_LINES, 0, 6);
        // glLineWidth(5.0f);

        glBindVertexArray(0);

        // End frame
        glfwSwapBuffers(window);

        // Detect inputs
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
    }

    // Shutdown GLFW
    glfwTerminate();

    return 0;
}


