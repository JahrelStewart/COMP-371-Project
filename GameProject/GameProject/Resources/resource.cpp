#include "../Headers/header.h"

//Function gets the shaders and determine if they can be compiled properly
static int setupVertexAndFragmentShader(GLuint shaderType, const std::string& shaderSRC) {
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
        std::cerr << shaderSource << std::endl;
    }

    return shader;
}

//Function that gets the vertex and fragment shader and checks to make sure the linking was successful
int compileAndLinkShaders(const std::string& vertexShaderSRC, const std::string& fragmentShaderSRC, const std::string* geometryShaderSRC) {
    // TODO
    // compile and link shader program
    // return shader program id
    // ------------------------------------

    // vertex shader
    int vertexShader = setupVertexAndFragmentShader(GL_VERTEX_SHADER, vertexShaderSRC);
    // fragment shader
    int fragmentShader = setupVertexAndFragmentShader(GL_FRAGMENT_SHADER, fragmentShaderSRC);

    int geometryShader;
    if (geometryShaderSRC != nullptr) {
        geometryShader = setupVertexAndFragmentShader(GL_GEOMETRY_SHADER, *geometryShaderSRC);
    }

    // link shaders
    int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    if (geometryShaderSRC != nullptr)
        glAttachShader(shaderProgram, geometryShader);
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
    if (geometryShaderSRC != nullptr)
        glDeleteShader(geometryShader);

    return shaderProgram;

}