// COMP 371 Project
// 20/08/2021

#include "../Headers/header.h"
#include "../Object/model.h"

//screen size settings:
unsigned int SCR_WIDTH = 1024;
unsigned int SCR_HEIGHT = 768;
const unsigned int shadowWidth = 1024, shadowHeight = 1024;

unsigned int depthMapObject, depthCubemap;
unsigned int gridTexture, wallTexture, cubeTexture, enclosingCubeTexture;

//These variables are essential for the creation of the grid wall, and text
GLuint wallInstances;
GLuint texts;

int gridSize = 100;
int totalTiles = (int)(pow(gridSize, 2.0) + 0.5);
int wallSize = 5;
int totalWallCubes = (int)(pow(wallSize, 2.0) + 0.5);
int wallCubeExclude = 0;
int wallCubeExclude1 = 0;
int wallCubeExclude2 = 0;
int wallCubeExclude3 = 0;
int amtCubeVertices = 0;
glm::vec3* quadPositons = new glm::vec3[10000];

//Camera essential properties:
glm::vec3 cameraPos = glm::vec3(0.0f, 40.0f, 80.0f);
glm::vec3 cameraLookAt = glm::vec3(0.0f, -0.5f, -1.0f);
glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraLookAt);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraRightDirection = glm::cross(cameraUp, cameraDirection);

//These varibales are essential to how the camera and models operate
float growModel = 1.005f;
float shrinkModel = 0.995f;
float rotateModel = 90.0f;
float moveModel = 0.2f;
glm::mat4 worldOrientMatrix = glm::mat4(1.0f);
float worldOrient = 0.5f;
glm::vec2 camPanTiltZoom(-90.0f, 0.0f), preCamPanTiltZoom(-90.0f, 0.0f);
glm::vec2 camWorldOrient(0.0f, 0.0f), preCamWorldOrient(0.0f, 0.0f);
float zoom = 55.0f, preZoom = 55.0f;
bool checkPos = true;
glm::vec2 lastPos(0.0f, 0.0f);
glm::mat4 lastWallMove = glm::mat4(1.0f);

//calculate view matrix..which is essentially the view of the camera        
glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraLookAt, cameraUp);
//create projection matrix
glm::mat4 projection = glm::perspective(glm::radians(zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
glm::mat4 textProjection = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT);

bool textureToggle = true;
bool shadowToggle = true;
bool isCube = false;
bool isLightSource = false;

glm::vec3 lightColor = glm::vec3((float)255 / 255, (float)255 / 255, (float)255 / 255);
glm::vec3 lightPos = glm::vec3(0.0f, 60.0f, 0.0f);

//Essential Variables for object rotate animation:
float seconds = 0.0f;
float moon = 0.0f;
float lastSecond = 0.0f;
int minutes = 1;
float lastTime = 0.0f;
float now = 0.0f;
float spin = false;
float spinAmount = 90.0f;
bool canSpin = false;
bool isLeftSpin = false;
bool isRightSpin = false;
bool isUpSpin = false;
bool isDownSpin = false;
bool firstTime = true;
bool occupied = false;
bool scoreChecked = false;

//Essential Variables for camera animation:
float cameraFirstAnimate = 30.0f;
float cameraSecondAnimate = 80.0f;
bool animationDone = false;
bool isCollision = false;
float lightDuration = 0.0f;
int option;
int shuffleRotation;
float rotations[4] = { 0.0f, 90.0f, 180.0f, 270.0f };

//player score
int score = 0;

//Struct that is used to keep track of which model is being focused on (by using the key 1 through 6)
struct PlayerInfo {
    glm::mat4 playerMove = glm::mat4(1.0f);
    glm::mat4 playerScale = glm::mat4(1.0f);
    glm::mat4 playerRotate = glm::mat4(1.0f);
};

PlayerInfo player;
PlayerInfo* currentPlayer = &player;

struct Models {
    std::vector<glm::vec3> wallPos;
    std::vector<glm::mat4> cubePos;
};

Models models[4];

struct Character {
    unsigned int TextureID;
    glm::ivec2   Size;
    glm::ivec2   Bearing;
    unsigned int Advance;
};

std::map<char, Character> Characters;

//Struct that is responsible storing each shader from the shaders file
struct Shaders {
    std::string GridShaderSource;
    std::string WallShaderSource;
    std::string CubeShaderSource;
    std::string ShadowMapShaderSource;
    std::string GeometryShaderSource;
    std::string ShadowFragmentShaderSource;
    std::string LightShadowTextureFragmentShaderSource;
    std::string TextVertexShaderSource;
    std::string TextFragmentShaderSource;
    std::string ModelVertexShaderSource;
    std::string ModelFragmentShaderSource;
};

//Function that actually does the parsing of the shader file. Since there are multiple shaders in one file, we read line by line, separate them and store them as properties within the Shaders struct 
static Shaders readShaders(const std::string& file) {
    std::ifstream stream(file);

    enum class whichShader {
        EMPTY = -1, GRIDSHADER = 0, WALLSHADER = 1, CUBESHADER = 2, SHADOWMAPSHADER = 3, GEOMETRYSHADER = 4, SHADOWFRAGMENTSHADER = 5, LIGHTSHADOWTEXTUREFRAGMENTSHADER = 6, TEXTVERTEX = 7, TEXTFRAGMENT = 8, MODELVERTEX = 9, MODELFRAGMENT = 10
    };

    std::stringstream shaderStream[11];
    std::string line;
    whichShader which = whichShader::EMPTY;

    while (getline(stream, line)) {
        if (line.find("#shader") != std::string::npos) {
            if (line.find("gridShader") != std::string::npos) {
                which = whichShader::GRIDSHADER;
            }
            else if (line.find("wallShader") != std::string::npos) {
                which = whichShader::WALLSHADER;
            }
            else if (line.find("cubeShader") != std::string::npos) {
                which = whichShader::CUBESHADER;
            }
            else if (line.find("shadowMapShader") != std::string::npos) {
                which = whichShader::SHADOWMAPSHADER;
            }
            else if (line.find("geometryShader") != std::string::npos) {
                which = whichShader::GEOMETRYSHADER;
            }
            else if (line.find("shadowFragmentShader") != std::string::npos) {
                which = whichShader::SHADOWFRAGMENTSHADER;
            }
            else if (line.find("fragmentLightShadowTextureShader") != std::string::npos) {
                which = whichShader::LIGHTSHADOWTEXTUREFRAGMENTSHADER;
            }
            else if (line.find("textVertex") != std::string::npos) {
                which = whichShader::TEXTVERTEX;
            }
            else if (line.find("textFragment") != std::string::npos) {
                which = whichShader::TEXTFRAGMENT;
            }
            else if (line.find("modelVertex") != std::string::npos) {
                which = whichShader::MODELVERTEX;
            }
            else if (line.find("modelFragment") != std::string::npos) {
                which = whichShader::MODELFRAGMENT;
            }
        }
        else {
            if (which != whichShader::EMPTY) {
                shaderStream[(int)which] << line << "\n";
            }
        }
    }

    return { shaderStream[0].str(), shaderStream[1].str(), shaderStream[2].str(), shaderStream[3].str(), shaderStream[4].str(), shaderStream[5].str(), shaderStream[6].str(), shaderStream[7].str(), shaderStream[8].str(),shaderStream[9].str(), shaderStream[10].str() };
}

//Function that is called within the glfwSetCursorPosCallback function to perform calculation from the current X & Y mouse positions
void mouseCalculations(GLFWwindow* window, double currentposX, double currentposY) {
    float currentXpos = (float)currentposX;
    float currentYpos = (float)currentposY;

    //When the function is ran for the first time, there is not last/previous position. Here we initialize the last positions to the current X & Y positions.
    if (checkPos) {
        lastPos.x = currentXpos;
        lastPos.y = currentYpos;
        checkPos = false;
    }

    //Uses the previous values and reassigns them to the current values. This prevents the camera from suddenly jumping to a new location when the user releases the mouse button and then clicks again.
    zoom = preZoom;
    camPanTiltZoom.x = preCamPanTiltZoom.x;
    camPanTiltZoom.y = preCamPanTiltZoom.y;
    camWorldOrient.x = preCamWorldOrient.x;
    camWorldOrient.y = preCamWorldOrient.y;

    //Calculates X & Y offset of camera and multiplies by constant to control speed 
    float offsetX = lastPos.x - currentXpos;
    camPanTiltZoom.x += offsetX * 0.15f;
    camWorldOrient.x += offsetX * 0.15f;

    float offsetY = lastPos.y - currentYpos;
    zoom += offsetY * 0.1f;

    float camMoveOffsetY = currentYpos - lastPos.y;
    camPanTiltZoom.y += camMoveOffsetY * 0.15f;
    camWorldOrient.y += camMoveOffsetY * 0.15f;

    //Clamps zoom
    if (zoom >= 150.0f) {
        zoom = 150.0f;
    }
    if (zoom <= 2.0f) {
        zoom = 2.0f;
    }

    //Clamps Camera pan, tilt and zoom
    camPanTiltZoom.y = std::max(-85.0f, std::min(85.0f, camPanTiltZoom.y));
    camWorldOrient.y = std::max(-85.0f, std::min(85.0f, camWorldOrient.y));

    //Update last positions so that camera movement will result in smooth transitions
    lastPos.x = currentXpos;
    lastPos.y = currentYpos;
}

//allows string to formatted so that single digits are represented as two digits eg. 2 -> 02
std::string to_format(const int number) {
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << number;
    return ss.str();
}

//Function that creates the Vertex Array Object, Vertex Buffer Objects and Vertices for all drawn objects
GLuint createVertexArrayObject() {
    //ground surafce vertex points
    float gridVertexArray[] = {
        //positions         colors												textures    Normals
        -0.5f, -0.5f, 0.0f,	170.0f / 255.0f, 185.0f / 255.0f, 207.0f / 255.0f,	0.0f, 0.0f,	0.0f, 0.0f, 1.0f,
        -0.5f, 0.5f, 0.0f,	170.0f / 255.0f, 185.0f / 255.0f, 207.0f / 255.0f,	0.0f, 1.0f,	0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.0f,	170.0f / 255.0f, 185.0f / 255.0f, 207.0f / 255.0f,	1.0f, 1.0f,	0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.0f,	170.0f / 255.0f, 185.0f / 255.0f, 207.0f / 255.0f,	1.0f, 0.0f,	0.0f, 0.0f, 1.0f,
    };

    //creating indexes for index buffer to avoid dupilicate vertices
    unsigned int gridIndexes[] = {
        0, 1, 2,
        2, 3, 0
    };

    //We create the 100 x 100 grid here to avoid calling the draw-call many times. This will boost performance	
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

    //Vertices for Wall
    float wallVertexArray[] = {
        //positions			//textures           
        -2.0f, -2.0f, -1.0f,  0.0f, 0.0f,  0.0f, 0.0f, -1.0f, //Back Face
         2.0f, -2.0f, -1.0f,  1.0f, 0.0f,  0.0f, 0.0f, -1.0f,
         2.0f,  2.0f, -1.0f,  1.0f, 1.0f,  0.0f, 0.0f, -1.0f,
         2.0f,  2.0f, -1.0f,  1.0f, 1.0f,  0.0f, 0.0f, -1.0f,
        -2.0f,  2.0f, -1.0f,  0.0f, 1.0f,  0.0f, 0.0f, -1.0f,
        -2.0f, -2.0f, -1.0f,  0.0f, 0.0f,  0.0f, 0.0f, -1.0f,

        -2.0f, -2.0f,  1.0f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f, //Front Face
         2.0f, -2.0f,  1.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,
         2.0f,  2.0f,  1.0f,  1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
         2.0f,  2.0f,  1.0f,  1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
        -2.0f,  2.0f,  1.0f,  0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
        -2.0f, -2.0f,  1.0f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f,

        -2.0f,  2.0f,  1.0f,  1.0f, 1.0f,  -1.0f, 0.0f, 0.0f, //Left Face
        -2.0f,  2.0f, -1.0f,  0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
        -2.0f, -2.0f, -1.0f,  0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
        -2.0f, -2.0f, -1.0f,  0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
        -2.0f, -2.0f,  1.0f,  1.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
        -2.0f,  2.0f,  1.0f,  1.0f, 1.0f,  -1.0f, 0.0f, 0.0f,

         2.0f,  2.0f,  1.0f,  1.0f, 1.0f,  1.0f, 0.0f, 0.0f, //Right Face
         2.0f,  2.0f, -1.0f,  0.0f, 1.0f,  1.0f, 0.0f, 0.0f,
         2.0f, -2.0f, -1.0f,  0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
         2.0f, -2.0f, -1.0f,  0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
         2.0f, -2.0f,  1.0f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f,
         2.0f,  2.0f,  1.0f,  1.0f, 1.0f,  1.0f, 0.0f, 0.0f,

        -2.0f, -2.0f, -1.0f,  0.0f, 1.0f,  0.0f, -1.0f, 0.0f, // Bottom Face
         2.0f, -2.0f, -1.0f,  1.0f, 1.0f,  0.0f, -1.0f, 0.0f,
         2.0f, -2.0f,  1.0f,  1.0f, 0.0f,  0.0f, -1.0f, 0.0f,
         2.0f, -2.0f,  1.0f,  1.0f, 0.0f,  0.0f, -1.0f, 0.0f,
        -2.0f, -2.0f,  1.0f,  0.0f, 0.0f,  0.0f, -1.0f, 0.0f,
        -2.0f, -2.0f, -1.0f,  0.0f, 1.0f,  0.0f, -1.0f, 0.0f,

        -2.0f,  2.0f, -1.0f,  0.0f, 1.0f,  0.0f, 1.0f, 0.0f, //Top Face
         2.0f,  2.0f, -1.0f,  1.0f, 1.0f,  0.0f, 1.0f, 0.0f,
         2.0f,  2.0f,  1.0f,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
         2.0f,  2.0f,  1.0f,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
        -2.0f,  2.0f,  1.0f,  0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
        -2.0f,  2.0f, -1.0f,  0.0f, 1.0f,  0.0f, 1.0f, 0.0f
    };

    //We create the 5x5 grid here to avoid calling the draw-call many times. This will boost performance   
    int wallIndex = 0;
    int wallIndex2 = 0;
    //offset centers all cubes around the origin for both X and Y planes
    float wallOffset = float(((wallSize * 4) / 2.0f) - 2.0f);
    //Draw each tile row by row on the XY plane
    for (int i = 0; i < (4 * wallSize); i = i + 4) {
        for (int j = 0; j < (4 * wallSize); j = j + 4) {

            if (((i >= 4 && i <= 8) && (j >= 4 && j <= 12)) || (i == 12 && j == 8)) {
                wallCubeExclude++;
            }
            else {
                glm::vec3 wallPos;
                wallPos.x = (float)(j - wallOffset);
                wallPos.y = (float)(i - wallOffset) + 20.0f;
                wallPos.z = 0.0f;
                models[0].wallPos.push_back(wallPos);
            }

            if (((i >= 4 && i <= 12) && j == 8) || (i == 8 && j == 4) || (i == 4 && j == 12)) {
                wallCubeExclude1++;
            }
            else {
                glm::vec3 wallPos;
                wallPos.x = (float)(j - wallOffset);
                wallPos.y = (float)(i - wallOffset) + 20.0f;
                wallPos.z = 0.0f;
                models[1].wallPos.push_back(wallPos);
            }

            if (((j >= 4 && j <= 12) && i == 8) || (i == 12 && j == 12)) {
                wallCubeExclude2++;
            }
            else {
                glm::vec3 wallPos;
                wallPos.x = (float)(j - wallOffset);
                wallPos.y = (float)(i - wallOffset) + 20.0f;
                wallPos.z = 0.0f;
                models[2].wallPos.push_back(wallPos);
            }

            if (((i >= 4 && i <= 8) && j == 8) || (i == 12 && j == 4) || (i == 12 && j == 12)) {
                wallCubeExclude3++;
            }
            else {
                glm::vec3 wallPos;
                wallPos.x = (float)(j - wallOffset);
                wallPos.y = (float)(i - wallOffset) + 20.0f;
                wallPos.z = 0.0f;
                models[3].wallPos.push_back(wallPos);
            }

        }
    }

    float cubesVertexArray[] = {
        //positions			//textures  //Normals        
        -2.0f, -2.0f, -2.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,	//Back Face
         2.0f, -2.0f, -2.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
         2.0f,  2.0f, -2.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
         2.0f,  2.0f, -2.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
        -2.0f,  2.0f, -2.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f,
        -2.0f, -2.0f, -2.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,

        -2.0f, -2.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,	//Front Face
         2.0f, -2.0f, 2.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
         2.0f,  2.0f, 2.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
         2.0f,  2.0f, 2.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -2.0f,  2.0f, 2.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -2.0f, -2.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,

        -2.0f,  2.0f,  2.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,	//Left Face
        -2.0f,  2.0f, -2.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        -2.0f, -2.0f, -2.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        -2.0f, -2.0f, -2.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        -2.0f, -2.0f,  2.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        -2.0f,  2.0f,  2.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,

         2.0f,  2.0f,  2.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,	//Right Face
         2.0f,  2.0f, -2.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
         2.0f, -2.0f, -2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
         2.0f, -2.0f, -2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
         2.0f, -2.0f,  2.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
         2.0f,  2.0f,  2.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,

        -2.0f, -2.0f, -2.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f,	// Bottom Face
         2.0f, -2.0f, -2.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
         2.0f, -2.0f,  2.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
         2.0f, -2.0f,  2.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        -2.0f, -2.0f,  2.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        -2.0f, -2.0f, -2.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f,

        -2.0f, 2.0f, -2.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,	//Top Face
         2.0f, 2.0f, -2.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
         2.0f, 2.0f,  2.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
         2.0f, 2.0f,  2.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -2.0f, 2.0f,  2.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -2.0f, 2.0f, -2.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f
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

    GLuint vertexArrayObject;
    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);

    // Grid
    //------------------------------------------------------------------------------------------------------------------------------------------

    GLuint grid;
    glGenBuffers(1, &grid);
    glBindBuffer(GL_ARRAY_BUFFER, grid);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gridVertexArray), gridVertexArray, GL_STATIC_DRAW);    

    glVertexAttribPointer(0,
        3,                                               
        GL_FLOAT,                                        
        GL_FALSE,                                        
        11 * sizeof(float),                              
        (void*)0                                         
    );
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1,
        3,
        GL_FLOAT,
        GL_FALSE,
        11 * sizeof(float),
        (void*)(3 * sizeof(float))
    );
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2,
        2,
        GL_FLOAT,
        GL_FALSE,
        11 * sizeof(float),
        (void*)(6 * sizeof(float))
    );
    glEnableVertexAttribArray(2);
    
    glVertexAttribPointer(3,
        3,
        GL_FLOAT,
        GL_FALSE,
        11 * sizeof(float),
        (void*)(8 * sizeof(float))
    );
    glEnableVertexAttribArray(3);

    GLuint gridInstances;
    glGenBuffers(1, &gridInstances);
    glBindBuffer(GL_ARRAY_BUFFER, gridInstances);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)* totalTiles, &quadPositons[0], GL_STATIC_DRAW);
    
    glVertexAttribPointer(4,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(glm::vec3),
        (void*)0
    );
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(4, 1);

    GLuint gridIndexBuffer;
    glGenBuffers(1, &gridIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gridIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), gridIndexes, GL_STATIC_DRAW);
        
    glGenTextures(1, &gridTexture);
    glBindTexture(GL_TEXTURE_2D, gridTexture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load("Resources/Textures/tileTexture.png", &width, &height, &nrChannels, 4);

    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Unable to Load Texture!" << std::endl;
    }

    stbi_image_free(data);    

    // Wall
    //------------------------------------------------------------------------------------------------------------------------------------------
    
    GLuint wall;
    glGenBuffers(1, &wall);
    glBindBuffer(GL_ARRAY_BUFFER, wall);
    glBufferData(GL_ARRAY_BUFFER, sizeof(wallVertexArray), wallVertexArray, GL_STATIC_DRAW);
    
    glVertexAttribPointer(5,
        3,
        GL_FLOAT,
        GL_FALSE,
        8 * sizeof(float),
        (void*)0
    );
    glEnableVertexAttribArray(5);
    
    glVertexAttribPointer(6,
        2,
        GL_FLOAT,
        GL_FALSE,
        8 * sizeof(float),
        (void*)(3 * sizeof(float))
    );
    glEnableVertexAttribArray(6);
    
    glVertexAttribPointer(7,
        3,
        GL_FLOAT,
        GL_FALSE,
        8 * sizeof(float),
        (void*)(5 * sizeof(float))
    );
    glEnableVertexAttribArray(7);
        
    glGenBuffers(1, &wallInstances);
    glBindBuffer(GL_ARRAY_BUFFER, wallInstances);
    glBufferData(GL_ARRAY_BUFFER, (totalWallCubes - wallCubeExclude) * sizeof(glm::vec3), &models[0].wallPos[0], GL_STATIC_DRAW);
    
    glVertexAttribPointer(8,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(glm::vec3),
        (void*)0
    );
    glEnableVertexAttribArray(8);
    glVertexAttribDivisor(8, 1);
           
    glGenTextures(1, &wallTexture);
    glBindTexture(GL_TEXTURE_2D, wallTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    stbi_set_flip_vertically_on_load(true);
    data = stbi_load("Resources/Textures/wallTexture.png", &width, &height, &nrChannels, 4);

    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Unable to Load Texture!" << std::endl;
    }

    stbi_image_free(data);

    //// Cube
    ////------------------------------------------------------------------------------------------------------------------------------------------
    
    GLuint cube;
    glGenBuffers(1, &cube);
    glBindBuffer(GL_ARRAY_BUFFER, cube);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubesVertexArray), cubesVertexArray, GL_STATIC_DRAW);
    
    glVertexAttribPointer(9,
        3,
        GL_FLOAT,
        GL_FALSE,
        8 * sizeof(float),
        (void*)0
    );
    glEnableVertexAttribArray(9);
    
    glVertexAttribPointer(10,
        2,
        GL_FLOAT,
        GL_FALSE,
        8 * sizeof(float),
        (void*)(3 * sizeof(float))
    );
    glEnableVertexAttribArray(10);
    
    glVertexAttribPointer(11,
        3,
        GL_FLOAT,
        GL_FALSE,
        8 * sizeof(float),
        (void*)(5 * sizeof(float))
    );
    glEnableVertexAttribArray(11);

    glGenTextures(1, &cubeTexture);
    glBindTexture(GL_TEXTURE_2D, cubeTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    stbi_set_flip_vertically_on_load(true);
    data = stbi_load("Resources/Textures/objectTexture.png", &width, &height, &nrChannels, 4);

    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Unable to Load Texture!" << std::endl;
    }

    stbi_image_free(data);

    glGenTextures(1, &enclosingCubeTexture);
    glBindTexture(GL_TEXTURE_2D, enclosingCubeTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    stbi_set_flip_vertically_on_load(true);
    data = stbi_load("Resources/Textures/night.png", &width, &height, &nrChannels, 4);

    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Unable to Load Texture!" << std::endl;
    }

    stbi_image_free(data);

    //Texts to be displayed on screen
    //------------------------------------------------------------------------------------------------------------------------------------------       
        
    glGenBuffers(1, &texts);
    glBindBuffer(GL_ARRAY_BUFFER, texts);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    
    glVertexAttribPointer(12,
        4,
        GL_FLOAT,
        GL_FALSE,
        4 * sizeof(float),
        (void*)0
    );
    glEnableVertexAttribArray(12);

    ////------------------------------------------------------------------------------------------------------------------------------------------

    glBindVertexArray(0);

    ////depth Map for point lights
    ////------------------------------------------------------------------------------------------------------------------------------------------

    glGenFramebuffers(1, &depthMapObject);
    glGenTextures(1, &depthCubemap);

    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, shadowHeight, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapObject);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //------------------------------------------------------------------------------------------------------------------------------------------

    return vertexArrayObject;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);

    SCR_HEIGHT = height;
    SCR_WIDTH = width;
    //Here we ensure that the aspect ratio remains consistent and that the meshes aren't deformed on screen resize
    projection = glm::perspective(glm::radians(zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
    textProjection = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // if (key == GLFW_KEY_X && action == GLFW_PRESS) {
    //     textureToggle = !textureToggle;
    // }

    // //toggles shadows
    // if (key == GLFW_KEY_B && action == GLFW_PRESS) {
    //     shadowToggle = !shadowToggle;
    // }

    //object rotation animation
    if (key == GLFW_KEY_W && action == GLFW_PRESS) {
        if (occupied == false) {
            canSpin = true;
            isLeftSpin = false;
            isRightSpin = false;
            isUpSpin = true;
            isDownSpin = false;
        }

    }

    if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        if (occupied == false) {
            canSpin = true;
            isLeftSpin = true;
            isRightSpin = false;
            isUpSpin = false;
            isDownSpin = false;
        }

    }

    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        if (occupied == false) {
            canSpin = true;
            isLeftSpin = false;
            isRightSpin = false;
            isUpSpin = false;
            isDownSpin = true;
        }

    }

    if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        if (occupied == false) {
            canSpin = true;
            isLeftSpin = false;
            isRightSpin = true;
            isUpSpin = false;
            isDownSpin = false;
        }

    }
}

bool CheckCollision(glm::vec3& one, glm::vec3& two) {
    float size = 2.0f;
    // collision x-axis?
    bool collisionX = one.x + size > two.x &&
        two.x + size > one.x;

    // collision y-axis?
    bool collisionY = one.y + size > two.y &&
        two.y + size > one.y;

    // collision z-axis?
    bool collisionZ = one.z + size + 1.0f > two.z &&
        two.z + size > one.z;

    return collisionX && collisionY && collisionZ;
}

void loadFonts(FT_Face& face) {
    FT_Set_Pixel_Sizes(face, 0, 48);

    // disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // load first 128 characters of ASCII set
    for (unsigned char c = 0; c < 128; c++) {
        // Load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)
        };
        Characters.insert(std::pair<char, Character>(c, character));
    }
}

void writeText(int& shader, std::string text, float x, float y, float scale, glm::vec3 color) {
    // activate corresponding render state	
    glUseProgram(shader);
    //projection used for displaying texts    
    glUniformMatrix4fv(glGetUniformLocation(shader, "textProjection"), 1, GL_FALSE, glm::value_ptr(textProjection));
    glUniform3f(glGetUniformLocation(shader, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory

        glBindBuffer(GL_ARRAY_BUFFER, texts);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glVertexAttribPointer(12, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);        

        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

int main(int argc, char* argv[]) {
    // Initialize GLFW and OpenGL version
    glfwInit();

    // Create Window and rendering context using GLFW, resolution is 800x600
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Comp371 - Project (Super Hypercube)", NULL, NULL);
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

    //---------------------------------------------------------------------------------------------------------
        //Initialize fonts and check to see if there were any errors
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }

    FT_Face face;
    if (FT_New_Face(ft, "Resources/Fonts/JetBrainsMono-Regular.ttf", 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return -1;
    }

    loadFonts(face);
    // clear FreeType resources
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    //---------------------------------------------------------------------------------------------------------
        // //Initialize sound library irrKlang
         ISoundEngine* SoundEngine = createIrrKlangDevice();
         //check for error
         if (!SoundEngine) {
             std::cout << "ERROR::IRRKLANG: Failed to load Sound Engine" << std::endl;
             return 0; // error starting up the engine
         }

         ISoundSource* backgroundSound = SoundEngine->addSoundSourceFromFile("Resources/Sounds/fairy.mp3");
         ISoundSource* defeatSound = SoundEngine->addSoundSourceFromFile("Resources/Sounds/loss.wav");
         backgroundSound->setDefaultVolume(0.2f); //adjust volume level
         defeatSound->setDefaultVolume(0.2f);

         SoundEngine->play2D(backgroundSound, true);
    //---------------------------------------------------------------------------------------------------------

        // load models      
         Model ourModel("Resources/Model/MOON.obj");

    //---------------------------------------------------------------------------------------------------------

    Shaders shader = readShaders("Shaders/shaders.shader");

    GLuint vertexArrayObject = createVertexArrayObject();

    models[0].cubePos = { glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 4.0f)),
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

    models[1].cubePos = { glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 4.0f)),
                          glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 4.0f, 0.0f)),
                          glm::translate(glm::mat4(1.0f), glm::vec3(4.0f, 4.0f, 0.0f)),
                          glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 8.0f)),
                          glm::translate(glm::mat4(1.0f), glm::vec3(-4.0f, 0.0f, 8.0f)),
                          glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -4.0f, 0.0f)),
                          glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -4.0f, -4.0f)),
    };

    models[2].cubePos = { glm::translate(glm::mat4(1.0f), glm::vec3(4.0f, 0.0f, 4.0f)),
                          glm::translate(glm::mat4(1.0f), glm::vec3(4.0f, -4.0f, 4.0f)),
                          glm::translate(glm::mat4(1.0f), glm::vec3(-4.0f, 0.0f, -4.0f)),
    };

    models[3].cubePos = { glm::translate(glm::mat4(1.0f), glm::vec3(-4.0f, 4.0f, 4.0f)),
                          glm::translate(glm::mat4(1.0f), glm::vec3(4.0f, 4.0f, 4.0f)),
                          glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 4.0f, -4.0f)),
                          glm::translate(glm::mat4(1.0f), glm::vec3(4.0f, -4.0f, 0.0f)),
    };

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouseCalculations);
    glfwSetKeyCallback(window, key_callback);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    int wallShaderProgram = compileAndLinkShaders(shader.WallShaderSource, shader.LightShadowTextureFragmentShaderSource);
    int cubeShaderProgram = compileAndLinkShaders(shader.CubeShaderSource, shader.LightShadowTextureFragmentShaderSource);
    int gridShaderProgram = compileAndLinkShaders(shader.GridShaderSource, shader.LightShadowTextureFragmentShaderSource);
    int shadowShaderProgram = compileAndLinkShaders(shader.ShadowMapShaderSource, shader.ShadowFragmentShaderSource, &shader.GeometryShaderSource);
    int textShaderProgram = compileAndLinkShaders(shader.TextVertexShaderSource, shader.TextFragmentShaderSource);
    int modelShaderProgram = compileAndLinkShaders(shader.ModelVertexShaderSource, shader.ModelFragmentShaderSource);

    // std::cout << glGetString(GL_VERSION);    
    // Entering Main Loop 
    while (!glfwWindowShouldClose(window)) {
        // Smokey Dark Blue background
        glClearColor((float)13 / 255, (float)19 / 255, (float)33 / 255, 1.0f);
        // Each frame, reset color of each pixel to glClearColor
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(vertexArrayObject);

        //get current time to be used to animate the camera:
        float thisTime = (float)glfwGetTime();
        if (firstTime) {
            lastTime = thisTime;
            firstTime = false;
            scoreChecked = false;

            srand(time(0));
            option = rand() % 4; //allows for random appearance of objects              

            shuffleRotation = rand() % 4; //allows for random orientation of objects
            player.playerRotate = glm::rotate(player.playerRotate, glm::radians(rotations[shuffleRotation]), glm::vec3(1.0f, 0.0f, 0.0f)); //random orientation around x axis

            shuffleRotation = rand() % 4;
            player.playerRotate = glm::rotate(player.playerRotate, glm::radians(rotations[shuffleRotation]), glm::vec3(0.0f, 1.0f, 0.0f)); //random orientation around y axis

            shuffleRotation = rand() % 4;
            player.playerRotate = glm::rotate(player.playerRotate, glm::radians(rotations[shuffleRotation]), glm::vec3(0.0f, 0.0f, 1.0f)); //random orientation around z axis            
            
        }

        now = thisTime - lastTime;
        seconds += now;
        moon += now;
        lastTime = thisTime;

        //animate camera:
        if (!animationDone) {
            now *= 10.0f;
            cameraFirstAnimate += now + (thisTime * 3.0f);
            cameraPos = glm::vec3(0.0f, 40.0f, cameraFirstAnimate);
            view = glm::lookAt(cameraPos, cameraPos + cameraLookAt, cameraUp);
        }

        if (cameraFirstAnimate >= 80.0f) {
            animationDone = true;
            now *= 3.0f;
            cameraSecondAnimate += -now;
            cameraPos = glm::vec3(0.0f, 40.0f, cameraSecondAnimate);
            view = glm::lookAt(cameraPos, cameraPos + cameraLookAt, cameraUp);
        }


        //
        //
//-----------------------------------------------------------------------------------------------------------------------------------
        // 1. First Pass: render depth of scene to texture (from light's perspective)

        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        float near_plane = 1.0f, far_plane = 1000.0f;
        lightProjection = glm::perspective(glm::radians(90.0f), (GLfloat)shadowWidth / (GLfloat)shadowHeight, near_plane, far_plane);
        lightView = glm::lookAt(lightPos, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;

        std::vector<glm::mat4> shadowProperties;
        shadowProperties.push_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowProperties.push_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowProperties.push_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
        shadowProperties.push_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
        shadowProperties.push_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowProperties.push_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));


        glViewport(0, 0, shadowWidth, shadowHeight);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapObject);
        glClear(GL_DEPTH_BUFFER_BIT);

        //player's First Pass Shadow Model Mapping:        

        //glm::mat4 adjustTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(-20.0f, 0.0f, 0.f));		
        glUseProgram(shadowShaderProgram);

        for (unsigned int i = 0; i < 6; ++i) {
            int shadowMatrices_location = glGetUniformLocation(shadowShaderProgram, ("shadowMatrices[" + std::to_string(i) + "]").c_str());
            glUniformMatrix4fv(shadowMatrices_location, 1, GL_FALSE, glm::value_ptr(shadowProperties[i]));
        }

        int model_location = glGetUniformLocation(shadowShaderProgram, "u_Model");
        int isGrid_location = glGetUniformLocation(shadowShaderProgram, "is_Grid");
        int is_Wall_location = glGetUniformLocation(shadowShaderProgram, "is_Wall");

        int farPlane_location_ = glGetUniformLocation(shadowShaderProgram, "far_plane");
        int lightPos_location = glGetUniformLocation(shadowShaderProgram, "lightPos");

        glUniform1f(farPlane_location_, far_plane);
        glUniform3f(lightPos_location, lightPos.x, lightPos.y, lightPos.z);

        //Draw the depth map Grid
        {
            //calculate model matrix..which is the transform of the Grid
            glm::mat4 model = worldOrientMatrix * glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); //Rotate grid 90 degrees so that it lays flat on the XZ plane			

            glUniform1i(isGrid_location, true);
            glUniform1i(is_Wall_location, false);

            glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model));

            glLineWidth(1.0f);
            if (textureToggle) {
                glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, totalTiles);
            }
            else {
                glDrawElementsInstanced(GL_LINE_LOOP, 5, GL_UNSIGNED_INT, nullptr, totalTiles);
            }
        }

        //Draw depth map Wall
        {
            glm::mat4 model = worldOrientMatrix;

            glUniform1i(isGrid_location, false);
            glUniform1i(is_Wall_location, true);

            glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model));

            glBindBuffer(GL_ARRAY_BUFFER, wallInstances);

            if (option == 0) {                
                glBufferData(GL_ARRAY_BUFFER, (totalWallCubes - wallCubeExclude) * sizeof(glm::vec3), &models[0].wallPos[0], GL_STATIC_DRAW);
                glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0 );
                glEnableVertexAttribArray(8);
                glVertexAttribDivisor(8, 1);

                glDrawArraysInstanced(GL_TRIANGLES, 0, amtCubeVertices, totalWallCubes - wallCubeExclude);
            }
            else if (option == 1) {                
                glBufferData(GL_ARRAY_BUFFER, (totalWallCubes - wallCubeExclude1) * sizeof(glm::vec3), &models[1].wallPos[0], GL_STATIC_DRAW);
                glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
                glEnableVertexAttribArray(8);
                glVertexAttribDivisor(8, 1);

                glDrawArraysInstanced(GL_TRIANGLES, 0, amtCubeVertices, totalWallCubes - wallCubeExclude1);
            }
            else if (option == 2) {                
                glBufferData(GL_ARRAY_BUFFER, (totalWallCubes - wallCubeExclude2) * sizeof(glm::vec3), &models[2].wallPos[0], GL_STATIC_DRAW);
                glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
                glEnableVertexAttribArray(8);
                glVertexAttribDivisor(8, 1);

                glDrawArraysInstanced(GL_TRIANGLES, 0, amtCubeVertices, totalWallCubes - wallCubeExclude2);
            }
            else if (option == 3) {                
                glBufferData(GL_ARRAY_BUFFER, (totalWallCubes - wallCubeExclude3) * sizeof(glm::vec3), &models[3].wallPos[0], GL_STATIC_DRAW);
                glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
                glEnableVertexAttribArray(8);
                glVertexAttribDivisor(8, 1);

                glDrawArraysInstanced(GL_TRIANGLES, 0, amtCubeVertices, totalWallCubes - wallCubeExclude3);
            }
        }

        //Draw depth map Cubes
        {
            glm::mat4 parentCube;

            for (size_t i = 0; i <= models[option].cubePos.size(); i++) {
                glm::mat4 model;
                if (i == 0) {
                    //Rotate animation:
                    if (canSpin == true) {
                        spin += 3.0;
                        spin = std::min(spin, spinAmount);
                        occupied = true;
                    }

                    if (isRightSpin) {
                        player.playerRotate = glm::rotate(player.playerRotate, glm::radians(-3.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                    }
                    else if (isLeftSpin) {
                        player.playerRotate = glm::rotate(player.playerRotate, glm::radians(3.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                    }
                    else if (isDownSpin) {
                        player.playerRotate = glm::rotate(player.playerRotate, glm::radians(-3.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                    }
                    else if (isUpSpin) {
                        player.playerRotate = glm::rotate(player.playerRotate, glm::radians(3.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                    }

                    model = worldOrientMatrix;
                    model *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 20.0f, 40.0f));
                    model *= glm::translate(player.playerMove, glm::vec3(0.0f, 0.0f, (float)glfwGetTime() * -3.0f));
                    model *= glm::inverse(player.playerRotate);

                    parentCube = model;
                }
                else {
                    model = parentCube;
                    model *= models[option].cubePos[i - 1];
                }

                glUniform1i(isGrid_location, false);
                glUniform1i(is_Wall_location, false);

                glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model));

                glDrawArrays(GL_TRIANGLES, 0, amtCubeVertices);

            }
        }

        glBindVertexArray(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //
        //
//------------------------------------------------------------------------------------------------------------------------------------------
        // 2. Second Pass: render scene as normal using the generated depth/shadow map  				

        // reset viewport
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(vertexArrayObject);

        //draw starry night skybox        
        {
            glm::mat4 model;

            optional: model = worldOrientMatrix * glm::scale(glm::mat4(1.0f), glm::vec3(50.0f, 50.0f, 50.0f));            

            glUseProgram(cubeShaderProgram);
            int model_location_MVP = glGetUniformLocation(cubeShaderProgram, "u_Model_View_Projection");
            int model_location = glGetUniformLocation(cubeShaderProgram, "u_Model");

            int model_location_Color = glGetUniformLocation(cubeShaderProgram, "u_Color");
            int model_location_lightColor = glGetUniformLocation(cubeShaderProgram, "lightColor");
            int isLightSource_location = glGetUniformLocation(cubeShaderProgram, "isLightSource");
            int lightPos_location = glGetUniformLocation(cubeShaderProgram, "lightPos");
            int texture_location = glGetUniformLocation(cubeShaderProgram, "customTexture");
            int textureToggle_location = glGetUniformLocation(cubeShaderProgram, "textureToggle");
            int shadowToggle_location = glGetUniformLocation(cubeShaderProgram, "shadows");
            int shadowMap_location = glGetUniformLocation(gridShaderProgram, "shadowMap");
            int isCube_location = glGetUniformLocation(cubeShaderProgram, "isCube");
            int viewPos_location = glGetUniformLocation(cubeShaderProgram, "viewPos");
            int farPlane_location_ = glGetUniformLocation(cubeShaderProgram, "far_plane");
            int isEnclosingCube_location = glGetUniformLocation(cubeShaderProgram, "isEnclosingCube");
            int lightDirection_location = glGetUniformLocation(cubeShaderProgram, "lightDirection");                  

            glm::mat4 transformation_result = projection * view * model;
            glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(model_location_MVP, 1, GL_FALSE, glm::value_ptr(transformation_result));

            glUniform1f(farPlane_location_, far_plane);
            glUniform1i(textureToggle_location, true);
            glUniform1i(shadowToggle_location, shadowToggle);
            glUniform1i(isLightSource_location, false);
            glUniform1i(isCube_location, true);
            glUniform1i(isEnclosingCube_location, true);

            glLineWidth(1.0f);
            //Color Cube Faces
            glm::vec3 color = glm::vec3((float)135 / 255, (float)206 / 255, (float)235 / 255);
            glUniform3f(model_location_Color, color.x, color.y, color.z);
            glUniform3f(model_location_lightColor, lightColor.x, lightColor.y, lightColor.z);
            glUniform3f(lightPos_location, lightPos.x, lightPos.y, lightPos.z);
            glUniform3f(viewPos_location, cameraPos.x, cameraPos.y, cameraPos.z);

            glUniform1i(texture_location, 1);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, enclosingCubeTexture);

            //Repeat texture effect
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            glDrawArrays(GL_TRIANGLES, 0, amtCubeVertices);
            
        }

        //Draw the Grid
        {
            //calculate model matrix..which is the transform of the Grid
            glm::mat4 model = worldOrientMatrix * glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); //Rotate grid 90 degrees so that it lays flat on the XZ plane

            glUseProgram(gridShaderProgram);
            int model_location_MVP = glGetUniformLocation(gridShaderProgram, "u_Model_View_Projection");
            int model_location = glGetUniformLocation(gridShaderProgram, "u_Model");

            int texture_location = glGetUniformLocation(gridShaderProgram, "customTexture");
            int model_location_lightColor = glGetUniformLocation(gridShaderProgram, "lightColor");
            int isLightSource_location = glGetUniformLocation(gridShaderProgram, "isLightSource");
            int lightPos_location = glGetUniformLocation(gridShaderProgram, "lightPos");
            int textureToggle_location = glGetUniformLocation(gridShaderProgram, "textureToggle");
            int shadowToggle_location = glGetUniformLocation(gridShaderProgram, "shadows");
            int isCube_location = glGetUniformLocation(gridShaderProgram, "isCube");
            int viewPos_location = glGetUniformLocation(gridShaderProgram, "viewPos");
            int shadowMap_location = glGetUniformLocation(gridShaderProgram, "shadowMap");
            int farPlane_location_ = glGetUniformLocation(gridShaderProgram, "far_plane");
            glUniform1f(farPlane_location_, far_plane);

            glUniform1i(shadowMap_location, 3);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

            glm::mat4 transformation_result = projection * view * model;
            glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(model_location_MVP, 1, GL_FALSE, glm::value_ptr(transformation_result));

            glUniform1i(textureToggle_location, textureToggle);
            glUniform1i(shadowToggle_location, shadowToggle);
            glUniform1i(isLightSource_location, false);
            glUniform1i(isCube_location, false);

            glUniform3f(model_location_lightColor, lightColor.x, lightColor.y, lightColor.z);
            glUniform3f(viewPos_location, cameraPos.x, cameraPos.y, cameraPos.z);
            glUniform3f(lightPos_location, lightPos.x, lightPos.y, lightPos.z);

            glLineWidth(1.0f);
            if (textureToggle) {
                glUniform1i(texture_location, 0);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, gridTexture);

                glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, totalTiles);
            }
            else {
                glDrawElementsInstanced(GL_LINE_LOOP, 5, GL_UNSIGNED_INT, nullptr, totalTiles);
            }

        }

        //Draw Wall
        {
            glm::mat4 model = worldOrientMatrix;

            glUseProgram(wallShaderProgram);
            int model_location_MVP = glGetUniformLocation(wallShaderProgram, "u_Model_View_Projection");
            int model_location = glGetUniformLocation(wallShaderProgram, "u_Model");

            int model_location_Color = glGetUniformLocation(wallShaderProgram, "u_Color");
            int model_location_lightColor = glGetUniformLocation(wallShaderProgram, "lightColor");
            int isLightSource_location = glGetUniformLocation(wallShaderProgram, "isLightSource");
            int lightPos_location = glGetUniformLocation(wallShaderProgram, "lightPos");
            int texture_location = glGetUniformLocation(wallShaderProgram, "customTexture");
            int textureToggle_location = glGetUniformLocation(wallShaderProgram, "textureToggle");
            int shadowToggle_location = glGetUniformLocation(wallShaderProgram, "shadows");
            int isCube_location = glGetUniformLocation(wallShaderProgram, "isCube");
            int viewPos_location = glGetUniformLocation(wallShaderProgram, "viewPos");
            int shadowMap_location = glGetUniformLocation(wallShaderProgram, "shadowMap");
            int farPlane_location_ = glGetUniformLocation(wallShaderProgram, "far_plane");

            glUniform1i(shadowMap_location, 5);
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

            glm::mat4 transformation_result = projection * view * model;
            glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(model_location_MVP, 1, GL_FALSE, glm::value_ptr(transformation_result));

            glUniform1f(farPlane_location_, far_plane);
            glUniform1i(textureToggle_location, textureToggle);
            glUniform1i(shadowToggle_location, shadowToggle);
            glUniform1i(isLightSource_location, false);
            glUniform1i(isCube_location, true);

            if (textureToggle) {
                glUniform1i(texture_location, 1);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, wallTexture);
            }

            glm::vec3 color = glm::vec3((float)102 / 255, (float)215 / 255, (float)209 / 255);
            glUniform3f(model_location_Color, color.x, color.y, color.z);
            glUniform3f(model_location_lightColor, lightColor.x, lightColor.y, lightColor.z);
            glUniform3f(lightPos_location, lightPos.x, lightPos.y, lightPos.z);
            glUniform3f(viewPos_location, cameraPos.x, cameraPos.y, cameraPos.z);

            glBindBuffer(GL_ARRAY_BUFFER, wallInstances);

            if (option == 0) {
                glBufferData(GL_ARRAY_BUFFER, (totalWallCubes - wallCubeExclude) * sizeof(glm::vec3), &models[0].wallPos[0], GL_STATIC_DRAW);
                glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
                glEnableVertexAttribArray(8);
                glVertexAttribDivisor(8, 1);

                glDrawArraysInstanced(GL_TRIANGLES, 0, amtCubeVertices, totalWallCubes - wallCubeExclude);
            }
            else if (option == 1) {
                glBufferData(GL_ARRAY_BUFFER, (totalWallCubes - wallCubeExclude1) * sizeof(glm::vec3), &models[1].wallPos[0], GL_STATIC_DRAW);
                glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
                glEnableVertexAttribArray(8);
                glVertexAttribDivisor(8, 1);

                glDrawArraysInstanced(GL_TRIANGLES, 0, amtCubeVertices, totalWallCubes - wallCubeExclude1);
            }
            else if (option == 2) {
                glBufferData(GL_ARRAY_BUFFER, (totalWallCubes - wallCubeExclude2) * sizeof(glm::vec3), &models[2].wallPos[0], GL_STATIC_DRAW);
                glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
                glEnableVertexAttribArray(8);
                glVertexAttribDivisor(8, 1);

                glDrawArraysInstanced(GL_TRIANGLES, 0, amtCubeVertices, totalWallCubes - wallCubeExclude2);
            }
            else if (option == 3) {
                glBufferData(GL_ARRAY_BUFFER, (totalWallCubes - wallCubeExclude3) * sizeof(glm::vec3), &models[3].wallPos[0], GL_STATIC_DRAW);
                glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
                glEnableVertexAttribArray(8);
                glVertexAttribDivisor(8, 1);

                glDrawArraysInstanced(GL_TRIANGLES, 0, amtCubeVertices, totalWallCubes - wallCubeExclude3);
            }

        }

        std::vector<glm::vec3> objObject;

        //Draw Cubes
        {
            glUseProgram(cubeShaderProgram);
            int shadowMap_location = glGetUniformLocation(cubeShaderProgram, "shadowMap");

            glUniform1i(shadowMap_location, 6);
            glActiveTexture(GL_TEXTURE6);
            glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

            glm::mat4 parentCube;

            for (size_t i = 0; i <= models[option].cubePos.size(); i++) {
                glm::mat4 model;
                if (i == 0) {
                    //Rotate animation:
                    if (canSpin == true) {
                        spin += 3.0;
                        spin = std::min(spin, spinAmount);
                        occupied = true;
                    }

                    if (isRightSpin) {
                        player.playerRotate = glm::rotate(player.playerRotate, glm::radians(-3.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                    }
                    else if (isLeftSpin) {
                        player.playerRotate = glm::rotate(player.playerRotate, glm::radians(3.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                    }
                    else if (isDownSpin) {
                        player.playerRotate = glm::rotate(player.playerRotate, glm::radians(-3.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                    }
                    else if (isUpSpin) {
                        player.playerRotate = glm::rotate(player.playerRotate, glm::radians(3.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                    }

                    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 20.0f, 40.0f));
                    model *= glm::translate(player.playerMove, glm::vec3(0.0f, 0.0f, (float)glfwGetTime() * -3.0f));
                    model *= glm::inverse(player.playerRotate);

                    if (spin >= spinAmount) {
                        canSpin = false;
                        isLeftSpin = false;
                        isRightSpin = false;
                        isUpSpin = false;
                        isDownSpin = false;
                        occupied = false;
                        spin = 0.0f;
                    }

                    parentCube = model;
                }
                else {
                    model = parentCube;
                    model *= models[option].cubePos[i - 1];
                }

                glm::vec3 scale;
                glm::quat rotation;
                glm::vec3 translation;
                glm::vec3 skew;
                glm::vec4 perspective;
                glm::decompose(model, scale, rotation, translation, skew, perspective);
                objObject.push_back(translation);

                model = worldOrientMatrix * model;

                int model_location_MVP = glGetUniformLocation(cubeShaderProgram, "u_Model_View_Projection");
                int model_location = glGetUniformLocation(cubeShaderProgram, "u_Model");

                int model_location_Color = glGetUniformLocation(cubeShaderProgram, "u_Color");
                int model_location_lightColor = glGetUniformLocation(cubeShaderProgram, "lightColor");
                int isLightSource_location = glGetUniformLocation(cubeShaderProgram, "isLightSource");
                int lightPos_location = glGetUniformLocation(cubeShaderProgram, "lightPos");
                int texture_location = glGetUniformLocation(cubeShaderProgram, "customTexture");
                int textureToggle_location = glGetUniformLocation(cubeShaderProgram, "textureToggle");
                int shadowToggle_location = glGetUniformLocation(cubeShaderProgram, "shadows");
                int isCube_location = glGetUniformLocation(cubeShaderProgram, "isCube");
                int viewPos_location = glGetUniformLocation(cubeShaderProgram, "viewPos");
                int farPlane_location_ = glGetUniformLocation(cubeShaderProgram, "far_plane");
                int isEnclosingCube_location = glGetUniformLocation(cubeShaderProgram, "isEnclosingCube");

                glm::mat4 transformation_result = projection * view * model;
                glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model));
                glUniformMatrix4fv(model_location_MVP, 1, GL_FALSE, glm::value_ptr(transformation_result));

                glUniform1f(farPlane_location_, far_plane);
                glUniform1i(textureToggle_location, textureToggle);
                glUniform1i(shadowToggle_location, shadowToggle);
                glUniform1i(isLightSource_location, false);
                glUniform1i(isEnclosingCube_location, false);
                glUniform1i(isCube_location, true);

                if (textureToggle) {
                    glUniform1i(texture_location, 2);
                    glActiveTexture(GL_TEXTURE2);
                    glBindTexture(GL_TEXTURE_2D, cubeTexture);
                }

                glm::vec3 color = glm::vec3((float)233 / 255, (float)230 / 255, (float)255 / 255);
                glUniform3f(model_location_Color, color.x, color.y, color.z);
                glUniform3f(model_location_lightColor, lightColor.x, lightColor.y, lightColor.z);
                glUniform3f(lightPos_location, lightPos.x, lightPos.y, lightPos.z);
                glUniform3f(viewPos_location, cameraPos.x, cameraPos.y, cameraPos.z);


                glDrawArrays(GL_TRIANGLES, 0, amtCubeVertices);

            }
        }

        //Draw imported Model
        {
            // render the loaded model
            glUseProgram(modelShaderProgram);
            glm::mat4 model = glm::mat4(1.0f);
            model *= worldOrientMatrix;
            model = glm::rotate(model, glm::radians(moon * 20.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // translate it down so it's at the center of the scene
            model = glm::translate(model, glm::vec3(20.0f, 30.0f, 0.0f)); // translate it down so it's at the center of the scene
            model = glm::scale(model, glm::vec3(0.8f, 0.8f, 0.8f));	// it's a bit too big for our scene, so scale it down

            int projection_location = glGetUniformLocation(modelShaderProgram, "projection");
            int view_location = glGetUniformLocation(modelShaderProgram, "view");
            int model_location = glGetUniformLocation(modelShaderProgram, "model");
            int model_location_Color = glGetUniformLocation(modelShaderProgram, "u_Color");
            int model_location_lightColor = glGetUniformLocation(modelShaderProgram, "lightColor");
            int lightPos_location = glGetUniformLocation(modelShaderProgram, "lightPos");
            int viewPos_location = glGetUniformLocation(modelShaderProgram, "viewPos");

            glUniformMatrix4fv(projection_location, 1, GL_FALSE, glm::value_ptr(projection));
            glUniformMatrix4fv(view_location, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model));        

            glm::vec3 color = glm::vec3((float)255 / 255, (float)233 / 255, (float)206 / 255);
            glUniform3f(model_location_Color, color.x, color.y, color.z);

            glUniform3f(model_location_lightColor, lightColor.x, lightColor.y, lightColor.z);
            glUniform3f(lightPos_location, lightPos.x, lightPos.y, lightPos.z);
            glUniform3f(viewPos_location, cameraPos.x, cameraPos.y, cameraPos.z);

            ourModel.Draw(modelShaderProgram);
        }        

        glBindVertexArray(vertexArrayObject);

//check for collisions:
        bool checkedOnce = false;
        bool notFit = false;
        for (size_t i = 0; i < models[option].wallPos.size(); i++) {
            for (size_t j = 0; j < objObject.size(); j++) {
                if (CheckCollision(models[option].wallPos[i], objObject[j])) {
                    isCollision = true;
                }

                if (!checkedOnce) {
                    if (objObject[j].z >= -3) {
                        notFit = true;
                        checkedOnce = true;
                    }
                }
            }
        }

        if (isCollision) {
            now *= 10.0f;
            lightDuration += now;
            lightColor = glm::vec3((float)199 / 255, (float)130 / 255, (float)131 / 255);
            if (!scoreChecked) {
                SoundEngine->play2D(defeatSound, false);
                scoreChecked = true;
            }
        }
        else if (!isCollision && !notFit) {
            now *= 10.0f;
            lightDuration += now;
            lightColor = glm::vec3((float)104 / 255, (float)176 / 255, (float)171 / 255);            

            notFit = false;

            if (!scoreChecked) {
                if (option == 0) {
                    score += 5;
                }
                else if (option == 1) {
                    score += 15;
                }
                else if (option == 2) {
                    score += 5;
                }
                else if (option == 3) {
                    score += 10;
                }

                SoundEngine->play2D("Resources/Sounds/win.wav", false);

                scoreChecked = true;
            }
        }

        if (lightDuration >= 40.0f) {
            lightDuration = 0.0f;
            lightColor = glm::vec3((float)255 / 255, (float)255 / 255, (float)255 / 255);

            animationDone = false;
            firstTime = true;
            cameraFirstAnimate = cameraPos.z;
            cameraSecondAnimate = 80.0f;
            now = 0.0f;
            currentPlayer->playerScale = glm::mat4(1.0f);
            currentPlayer->playerRotate = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            currentPlayer->playerMove = glm::mat4(1.0f);
            glfwSetTime(0);
            isCollision = false;
        }

        //render text onto screen
        glm::vec3 textColour(255.0f / 255.0f, 251.0f / 255.0f, 250.0f / 255.0f);
        writeText(textShaderProgram, "SCORE:", 25.0f, (float)SCR_HEIGHT - 40.0f, 0.8f, textColour);
        writeText(textShaderProgram, std::to_string(score), 25.0f, (float)SCR_HEIGHT - 80.0f, 0.8f, textColour); //score

        writeText(textShaderProgram, "TIME lEFT:", (float)SCR_WIDTH - 260.0f, (float)SCR_HEIGHT - 40.0f, 0.8f, textColour);
        writeText(textShaderProgram, to_format(minutes) + ":" + to_format(59 - (int)seconds), (float)SCR_WIDTH - 260.0f, (float)SCR_HEIGHT - 80.0f, 0.8f, textColour);

        //decrement minute:
        if (int(seconds) != int(lastSecond)) {
            if ((int)seconds == 60) {
                --minutes;
                seconds = 0.0f;
            }
        }

        //game ends naturally due to there being no time left. options to exit? ore restart game?
        if (minutes == 0 && (59 - (int)seconds == 0)) {
            //glfwSetWindowShouldClose(window, true);
            seconds = 0.0f;
            minutes = 1;
            moon = 0.0f;
            score = 0;

            lightDuration = 0.0f;
            lightColor = glm::vec3((float)255 / 255, (float)255 / 255, (float)255 / 255);

            animationDone = false;
            firstTime = true;
            cameraFirstAnimate = cameraPos.z;
            cameraSecondAnimate = 80.0f;
            now = 0.0f;
            currentPlayer->playerScale = glm::mat4(1.0f);
            currentPlayer->playerRotate = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            currentPlayer->playerMove = glm::mat4(1.0f);
            glfwSetTime(0);
            isCollision = false;
        }

        lastSecond = seconds;
       
        glBindVertexArray(0);

        // End frame
        glfwSwapBuffers(window);

        // Detect inputs
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);         

    }

    currentPlayer = nullptr;
    delete currentPlayer;

    delete[] quadPositons;

    glDeleteProgram(shadowShaderProgram);
    glDeleteProgram(wallShaderProgram);
    glDeleteProgram(cubeShaderProgram);
    glDeleteProgram(gridShaderProgram);
    glDeleteProgram(textShaderProgram);
    glDeleteProgram(modelShaderProgram);    

    // Shutdown GLFW
    glfwTerminate();

    return 0;
}

