//
// COMP 371 Labs Framework
//
// Created by Nicolas Bergeron on 20/06/2019.
//

#include <iostream>
#include <list>

#define GLEW_STATIC 1   // This allows linking with Static Library on Windows, without DLL
#include <GL/glew.h>    // Include GLEW - OpenGL Extension Wrangler

#include <GLFW/glfw3.h> // cross-platform interface for creating a graphical context,
                        // initializing OpenGL and binding inputs

#include <glm/glm.hpp>  // GLM is an optimized math library with syntax to similar to OpenGL Shading Language
#include <glm/gtc/matrix_transform.hpp> // include this to create transformation matrices
#include <glm/common.hpp>


using namespace glm;
using namespace std;

/*
class Projectile
{
public:
    Projectile(vec3 position, vec3 velocity, int shaderProgram) : mPosition(position), mVelocity(velocity)
    {
        mWorldMatrixLocation = glGetUniformLocation(shaderProgram, "worldMatrix");

    }

    void Update(float dt)
    {
        mPosition += mVelocity * dt;
    }

    void Draw() {
        // this is a bit of a shortcut, since we have a single vbo, it is already bound
        // let's just set the world matrix in the vertex shader

        mat4 worldMatrix =
            (mat4(1.0f), mPosition) * rotate(mat4(1.0f), radians(180.0f), vec3(0.0f, 1.0f, 0.0f)) * scale(mat4(1.0f), vec3(0.2f, 0.2f, 0.2f));
        glUniformMatrix4fv(mWorldMatrixLocation, 1, GL_FALSE, &worldMatrix[0][0]);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    

public:
    GLuint mWorldMatrixLocation;
    vec3 mPosition;
    vec3 mVelocity;
};

*/

const char* getVertexShaderSource();

const char* getFragmentShaderSource();

int compileAndLinkShaders();

int createVertexArrayObject();

bool initContext();

void drawJohnObj(int shader, int V);

//int compileAndLinkShadersGrid();

/*
int createGrid() {

    vec3 groundVertices[] = {

       vec3(-0.5f, -0.8f, 0),
       vec3(-0.4f, -0.8f, 0),
       vec3(-0.3f,-0.8f, 0),
       vec3(-0.2f, -0.8f, 0),
       vec3(-0.1f, -0.8f, 0),
       vec3(0, -0.8f, 0),
       vec3(0.1f, -0.8f, 0),
       vec3(0.2f, -0.8f, 0),
       vec3(0.3f, -0.8f, 0),
       vec3(0.4f, -0.8f, 0)

    };

    GLuint vertexArrayObject1;
    glGenVertexArrays(1, &vertexArrayObject1);
    glBindVertexArray(vertexArrayObject1);


    GLuint vertexBufferObject1;
    glGenBuffers(1, &vertexBufferObject1);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);


    glVertexAttribPointer(0,                   // attribute 0 matches aPos in Vertex Shader
        1,                   // size
        GL_FLOAT,            // type
        GL_FALSE,            // normalized?
        1 * sizeof(vec3), // stride - each vertex contain 2 vec3 (position, color)
        (void*)0             // array buffer offset
    );


    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject1);
    glBindVertexArray(0);



    glEnableVertexAttribArray(0);


    return vertexArrayObject1;


}
*/

GLFWwindow* window = NULL;


int main(int argc, char* argv[])
{
    if (!initContext()) return -1;

    // Black background
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);

    // Compile and link shaders here ...
  


    // Camera parameters for view transform
    vec3 cameraPosition(0.0f, 0.0f, 10.0f);
    vec3 cameraLookAt(0.0f, 0.0f, -1.0f);
    vec3 cameraUp(0.0f, 1.0f, 0.0f);

    // Other camera parameters
    float cameraSpeed = 1.0f;
    float cameraFastSpeed = 2 * cameraSpeed;
    float cameraHorizontalAngle = 90.0f;
    float cameraVerticalAngle = 0.0f;
    bool  cameraFirstPerson = true; // press 1 or 2 to toggle this variable

    // Spinning cube at camera position
    float spinningCubeAngle = 0.0f;

    // Set projection matrix for shader, this won't change
    mat4 projectionMatrix = glm::perspective(70.0f,            // field of view in degrees
        800.0f / 600.0f,  // aspect ratio
        0.01f, 100.0f);   // near and far (near > 0)
   
    
    int shaderProgram = compileAndLinkShaders();
    glUseProgram(shaderProgram);
   
    
    GLuint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
    
    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

    // Set initial view matrix
    mat4 viewMatrix = lookAt(cameraPosition,  // eye
        cameraPosition + cameraLookAt,  // center
        cameraUp); // up

    GLuint viewMatrixLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
    glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
    glUseProgram(0);


    // Define and upload geometry to the GPU here ...
    

   //int vao1 = createGrid();

    // For frame time
    float lastFrameTime = glfwGetTime();
    int lastMouseLeftState = GLFW_RELEASE;
    double lastMousePosX, lastMousePosY;
    glfwGetCursorPos(window, &lastMousePosX, &lastMousePosY);

    // Other OpenGL states to set once before the Game Loop
    // Enable Backface culling
    //glEnable(GL_CULL_FACE);
   // glEnable(GL_FRONT_AND_BACK, GL_LINE);
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // @TODO 1 - Enable Depth Test
    // ...
   
  glEnable(GL_DEPTH_TEST);

    // Container for projectiles to be implemented in tutorial
   // list<Projectile> projectileList;


    // Entering Game Loop
    while (!glfwWindowShouldClose(window))
    {


        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Frame time calculation
       float dt = glfwGetTime() - lastFrameTime;
        lastFrameTime += dt;

        // Each frame, reset color of each pixel to glClearColor

        // @TODO 1 - Clear Depth Buffer Bit as well
        //glClear(GL_COLOR_BUFFER_BIT);

        
        // Draw geometry
        int vao = createVertexArrayObject();

       // int shad = shader;

        //int vao = V;
        glUseProgram(shaderProgram);

        glBindVertexArray(vao);



        // We can set the shader once, since we have only one

        
        
        //glBindBuffer(GL_ARRAY_BUFFER, vbo);

        
        drawJohnObj(shaderProgram, vao);
        
        // Draw ground

        
        //mat4 groundWorldMatrix = translate(mat4(1.0f), vec3(0.0f, -0.01f, 0.0f)) * scale(mat4(1.0f), vec3(1.0f, 1.0f, 1.0f));

       // mat4 groundWorldMatrix = translate(mat4(1.0f), vec3(0.0f, -0.01f, 0.0f)) * scale(mat4(1.0f), vec3(1000.0f, 0.02f, 1000.0f));
        
        //glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &groundWorldMatrix[0][0]);

       //glDrawArrays(GL_TRIANGLES, 0, 36); // 36 vertices, starting at index 0
        
        // Draw pillars

        
      //  mat4 pillarWorldMatrix = translate(mat4(1.0f), vec3(0.0f, 10.0f, 0.0f)) * scale(mat4(1.0f), vec3(1.0f, 1.0f, 1.0f));
       
        //glBindVertexArray(0);

        
      
       
       /*
       cube = translate(mat4(1.0f), vec3(2.0f, -3.0f, -3.0f));
       cube = cube * scale(mat4(1.0f), vec3(5.0f, 3.0f, 3.0f));
       glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
       glDrawArrays(GL_TRIANGLES, 0, 36);
       */



       //draw grid
       /*
        int shaderProgram = compileAndLinkShadersGrid();

       glUseProgram(shaderProgram);

       glBindVertexArray(vao1);

       //pass a color

       glm::vec3 color = glm::vec3((float)190 / 255, (float)61 / 255, (float)108 / 255);

       int model_location_Color = glGetUniformLocation(shaderProgram, "u_gridColor");
       
       glUniform3f(model_location_Color, color.x, color.y, color.z);

       glDrawArrays(GL_LINE, 0, 10);

       glBindVertexArray(0);

       glfwSwapBuffers(window);
       


       */



       



       //glDrawArrays(GL_LINE, 0, 10);
        

       //mat4 groundMatrix = translate(mat4(1.0f), vec3(0.0f, 10.0f, 0.0f)) * scale(mat4(1.0f), vec3(1.0f, 1.0f, 1.0f));

       //bool rotate = false;


       //draw the grid
       /*
        for (int i = 0; i < 2; ++i)
        {
            for (int j = 0; j < 100; ++j)
            {
                groundMatrix = translate(mat4(1.0f), vec3(-100.0f + i * 10.0f, 5.0f, -100.0f + j * 10.0f)) * scale(mat4(1.0f), vec3(1.0f, 10.0f, 1.0f));
               glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &groundMatrix[0][0]);
                glDrawArrays(GL_LINE, 0, 36);

                //pillarWorldMatrix = translate(mat4(1.0f), vec3(-100.0f + i * 10.0f, 0.55f, -100.0f + j * 10.0f)) * rotate(mat4(1.0f), radians(180.0f), vec3(0.0f, 1.0f, 0.0f)) * scale(mat4(1.0f), vec3(1.1f, 1.1f, 1.1f));
                //glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &pillarWorldMatrix[0][0]);
               // glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }
        */


       //mat4 wallMatrix; //= translate(mat4(1.0f), vec3(0.0f, 0.0f, 5.0f)) * scale(mat4(1.0f), vec3(0.5f, 0.5f, 0.5f));;

      // bool nextRow = false;

       //draw the wall

       
       /*
       for (int i = 0; i < 10; ++i)
       {
           for (int j = 0; j < 10; ++j)
           {
               //groundMatrix = translate(mat4(1.0f), vec3(-100.0f + i * 10.0f, 5.0f, -100.0f + j * 10.0f)) * scale(mat4(1.0f), vec3(1.0f, 10.0f, 1.0f));
               
               wallMatrix = translate(mat4(1.0f), vec3(-5.0f + j, -5.0f + i, -8.0f));;
               //*rotate(mat4(1.0f),radians(90.0f), vec3(1.0f,0.0f,0.0f)); //* scale(mat4(1.0f), vec3(1.0f, 10.0f, 1.0f));
               
               glUniform1i(decidedLocation,0);
               glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &wallMatrix[0][0]);
               glDrawArrays(GL_TRIANGLES, 0, 36);
               //glDrawArrays(GL_LINE, 0, 36);
               
               
               //pillarWorldMatrix = translate(mat4(1.0f), vec3(-100.0f + i * 10.0f, 0.55f, -100.0f + j * 10.0f)) * rotate(mat4(1.0f), radians(180.0f), vec3(0.0f, 1.0f, 0.0f)) * scale(mat4(1.0f), vec3(1.1f, 1.1f, 1.1f));
               //glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &pillarWorldMatrix[0][0]);
              // glDrawArrays(GL_TRIANGLES, 0, 36);
           }

       }
       */
       

       //draw the grid
       
       
       /*
       mat4 groundMatrix;

       for (int i = 0; i < 10; ++i)
       {
           for (int j = 0; j < 10; ++j)
           {
               //groundMatrix = translate(mat4(1.0f), vec3(-100.0f + i * 10.0f, 5.0f, -100.0f + j * 10.0f)) * scale(mat4(1.0f), vec3(1.0f, 10.0f, 1.0f));

               wallMatrix = translate(mat4(1.0f), vec3(-5.0f + j, -5.0f + i, -8.0f));;
               //*rotate(mat4(1.0f),radians(90.0f), vec3(1.0f,0.0f,0.0f)); //* scale(mat4(1.0f), vec3(1.0f, 10.0f, 1.0f));
               glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &wallMatrix[0][0]);
               glDrawArrays(GL_TRIANGLES, 0, 36);
               //glDrawArrays(GL_LINE, 0, 36);

               //pillarWorldMatrix = translate(mat4(1.0f), vec3(-100.0f + i * 10.0f, 0.55f, -100.0f + j * 10.0f)) * rotate(mat4(1.0f), radians(180.0f), vec3(0.0f, 1.0f, 0.0f)) * scale(mat4(1.0f), vec3(1.1f, 1.1f, 1.1f));
               //glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &pillarWorldMatrix[0][0]);
              // glDrawArrays(GL_TRIANGLES, 0, 36);
           }

       }
        */
        // @TODO 2B - Update and draw projectiles
       /*
        for (list<Projectile>::iterator it = projectileList.begin(); it != projectileList.end(); it++) {
            it->Update(dt);
            it->Draw();
        }
        */

       
        // Spinning cube at camera position
        spinningCubeAngle += 180.0f * dt;

        // @TODO 7 - Draw in view space for first person camera

        {
            // In third person view, let's draw the spinning cube in world space, like any other models
            mat4 spinningCubeWorldMatrix = translate(mat4(1.0f), cameraPosition) *
                rotate(mat4(1.0f), radians(spinningCubeAngle), vec3(0.0f, 1.0f, 0.0f)) *
                scale(mat4(1.0f), vec3(0.1f, 0.1f, 0.1f));

            //glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &spinningCubeWorldMatrix[0][0]);
        }
       // glDrawArrays(GL_TRIANGLES, 0, 36);


        glBindVertexArray(0);

        // End Frame
        glfwSwapBuffers(window);
        glfwPollEvents();

        // Handle inputs
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) // move camera down
        {
            cameraFirstPerson = true;
        }

        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) // move camera down
        {
            cameraFirstPerson = false;
        }


        // This was solution for Lab02 - Moving camera exercise
        // We'll change this to be a first or third person camera
        bool fastCam = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
        float currentCameraSpeed = (fastCam) ? cameraFastSpeed : cameraSpeed;


        // @TODO 4 - Calculate mouse motion dx and dy
        //         - Update camera horizontal and vertical angle


        // Please understand the code when you un-comment it!
        /*
        double mousePosX, mousePosY;
        glfwGetCursorPos(window, &mousePosX, &mousePosY);

        double dx = mousePosX - lastMousePosX;
        double dy = mousePosY - lastMousePosY;

        lastMousePosX = mousePosX;
        lastMousePosY = mousePosY;

        // Convert to spherical coordinates
        const float cameraAngularSpeed = 60.0f;
        cameraHorizontalAngle -= dx * cameraAngularSpeed * dt;
        cameraVerticalAngle   -= dy * cameraAngularSpeed * dt;

        // Clamp vertical angle to [-85, 85] degrees
        cameraVerticalAngle = std::max(-85.0f, std::min(85.0f, cameraVerticalAngle));
        if (cameraHorizontalAngle > 360)
        {
            cameraHorizontalAngle -= 360;
        }
        else if (cameraHorizontalAngle < -360)
        {
            cameraHorizontalAngle += 360;
        }

        float theta = radians(cameraHorizontalAngle);
        float phi = radians(cameraVerticalAngle);

        cameraLookAt = vec3(cosf(phi)*cosf(theta), sinf(phi), -cosf(phi)*sinf(theta));
        vec3 cameraSideVector = glm::cross(cameraLookAt, vec3(0.0f, 1.0f, 0.0f));

        glm::normalize(cameraSideVector);
         */

         // @TODO 5 = use camera lookat and side vectors to update positions with ASDW
         // adjust code below

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) // move camera to the left
        {
            cameraPosition.x -= currentCameraSpeed * dt;
        }

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) // move camera to the right
        {
            cameraPosition.x += currentCameraSpeed * dt;
        }

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) // move camera up
        {
            cameraPosition.y -= currentCameraSpeed * dt;
        }

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) // move camera down
        {
            cameraPosition.y += currentCameraSpeed * dt;
        }

        


        // TODO 6
        // Set the view matrix for first and third person cameras
        // - In first person, camera lookat is set like below
        // - In third person, camera position is on a sphere looking towards center
        mat4 viewMatrix = lookAt(cameraPosition, cameraPosition + cameraLookAt, cameraUp);

        GLuint viewMatrixLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
        glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);


        // @TODO 2A - Shoot Projectiles
        //
        // shoot projectiles on mouse left click
        // To detect onPress events, we need to check the last state and the current state to detect the state change
        // Otherwise, you would shoot many projectiles on each mouse press
    // ...

        

    }
    

    // Shutdown GLFW
    glfwTerminate();

    return 0;
}

const char* getVertexShaderSource()
{
    // For now, you use a string for your shader code, in the assignment, shaders will be stored in .glsl files
    return
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;"
        "layout (location = 1) in vec3 aColor;"
        ""
        "uniform mat4 worldMatrix;"
        "uniform mat4 viewMatrix;"  // default value for view matrix (identity)
        "uniform mat4 projectionMatrix;"
        ""
        "out vec3 vertexColor;"
        "void main()"
        "{"
        "   vertexColor = aColor;"
        "   mat4 modelViewProjection = projectionMatrix * viewMatrix * worldMatrix;"
        "   gl_Position = modelViewProjection * vec4(aPos.x, aPos.y, aPos.z, 1.0);"
        "}";
}

const char* getFragmentShaderSource()
{
    return
        "#version 330 core\n"
        "in vec3 vertexColor;"
        "out vec4 FragColor;"
        "uniform int decided_color;"
        "void main()"
        "{"
        "if (decided_color ==1)"
        "{"
        "FragColor = vec4(0.0f,0.0f,1.0f, 1.0f);"
        "} "
         "else"
        "{"
            "FragColor = vec4(vertexColor, 1.0f);"
         "}"
        
        "}";
}

/*
const char* getFragmentShaderSource1()
{
    return
        "#version 330 core\n"
        "uniform vec3 u_gridColor;"
        "out vec4 FragColor;"
        "void main()"
        "{"
        "   FragColor = vec4(u_gridColor.r, u_gridColor.g, u_gridColor.b, 1.0f);"
        "}";
}
*/

/*
int compileAndLinkShadersGrid()
{
    // compile and link shader program
    // return shader program id
    // ------------------------------------

    // vertex shader
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertexShaderSource = getVertexShaderSource();
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // fragment shader
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragmentShaderSource = getFragmentShaderSource();
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // link shaders
    int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

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
*/





int compileAndLinkShaders()
{
    // compile and link shader program
    // return shader program id
    // ------------------------------------

    // vertex shader
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertexShaderSource = getVertexShaderSource();
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // fragment shader
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragmentShaderSource = getFragmentShaderSource();
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // link shaders
    int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

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

int createVertexArrayObject()
{
    // Cube model
    vec3 vertexArray[] = {  // position,                            color
        vec3(-0.5f,-0.5f,-0.5f), vec3(1.0f, 0.0f, 0.0f), //left - red
        vec3(-0.5f,-0.5f, 0.5f), vec3(1.0f, 0.0f, 0.0f),
        vec3(-0.5f, 0.5f, 0.5f), vec3(1.0f, 0.0f, 0.0f),

        vec3(-0.5f,-0.5f,-0.5f), vec3(1.0f, 0.0f, 0.0f),
        vec3(-0.5f, 0.5f, 0.5f), vec3(1.0f, 0.0f, 0.0f),
        vec3(-0.5f, 0.5f,-0.5f), vec3(1.0f, 0.0f, 0.0f),

        vec3(0.5f, 0.5f,-0.5f), vec3(1.0f, 0.0f, 0.0f), // far - blue
        vec3(-0.5f,-0.5f,-0.5f), vec3(1.0f, 0.0f, 0.0f),
        vec3(-0.5f, 0.5f,-0.5f), vec3(1.0f, 0.0f, 0.0f),

        vec3(0.5f, 0.5f,-0.5f), vec3(1.0f, 0.0f, 0.0f),
        vec3(0.5f,-0.5f,-0.5f), vec3(1.0f, 0.0f, 0.0f),
        vec3(-0.5f,-0.5f,-0.5f), vec3(1.0f, 0.0f, 0.0f),

        vec3(0.5f,-0.5f, 0.5f), vec3(1.0f, 0.0f, 0.0f), // bottom - turquoise
        vec3(-0.5f,-0.5f,-0.5f), vec3(1.0f, 0.0f, 0.0f),
        vec3(0.5f,-0.5f,-0.5f), vec3(1.0f, 0.0f, 0.0f),

        vec3(0.5f,-0.5f, 0.5f), vec3(1.0f, 0.0f, 0.0f),
        vec3(-0.5f,-0.5f, 0.5f), vec3(1.0f, 0.0f, 0.0f),
        vec3(-0.5f,-0.5f,-0.5f), vec3(1.0f, 0.0f, 0.0f),

        vec3(-0.5f, 0.5f, 0.5f), vec3(1.0f, 0.0f, 0.0f), // near - green
        vec3(-0.5f,-0.5f, 0.5f), vec3(1.0f, 0.0f, 0.0f),
        vec3(0.5f,-0.5f, 0.5f), vec3(1.0f, 0.0f, 0.0f),

        vec3(0.5f, 0.5f, 0.5f), vec3(1.0f, 0.0f, 0.0f),
        vec3(-0.5f, 0.5f, 0.5f), vec3(1.0f, 0.0f, 0.0f),
        vec3(0.5f,-0.5f, 0.5f), vec3(1.0f, 0.0f, 0.0f),

        vec3(0.5f, 0.5f, 0.5f), vec3(1.0f, 0.0f, 0.0f), // right - purple
        vec3(0.5f,-0.5f,-0.5f), vec3(1.0f, 0.0f, 0.0f),
        vec3(0.5f, 0.5f,-0.5f), vec3(1.0f, 0.0f, 0.0f),

        vec3(0.5f,-0.5f,-0.5f), vec3(1.0f, 0.0f, 0.0f),
        vec3(0.5f, 0.5f, 0.5f), vec3(1.0f, 0.0f, 0.0f),
        vec3(0.5f,-0.5f, 0.5f), vec3(1.0f, 0.0f, 0.0f),

        vec3(0.5f, 0.5f, 0.5f), vec3(1.0f, 0.0f, 0.0f), // top - yellow
        vec3(0.5f, 0.5f,-0.5f), vec3(1.0f, 0.0f, 0.0f),
        vec3(-0.5f, 0.5f,-0.5f), vec3(1.0f, 0.0f, 0.0f),

        vec3(0.5f, 0.5f, 0.5f), vec3(1.0f, 0.0f, 0.0f),
        vec3(-0.5f, 0.5f,-0.5f), vec3(1.0f, 0.0f, 0.0f),
        vec3(-0.5f, 0.5f, 0.5f), vec3(1.0f, 0.0f, 0.0f)
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

    glVertexAttribPointer(0,                   // attribute 0 matches aPos in Vertex Shader
        3,                   // size
        GL_FLOAT,            // type
        GL_FALSE,            // normalized?
        2 * sizeof(vec3), // stride - each vertex contain 2 vec3 (position, color)
        (void*)0             // array buffer offset
    );
    glEnableVertexAttribArray(0);


    glVertexAttribPointer(1,                            // attribute 1 matches aColor in Vertex Shader
        3,
        GL_FLOAT,
        GL_FALSE,
        2 * sizeof(vec3),
        (void*)sizeof(vec3)      // color is offseted a vec3 (comes after position)
    );
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glBindVertexArray(0);


    
    //glEnableVertexAttribArray(0);


    return vertexArrayObject;
}



void drawJohnObj(int shader, int V) {

    

    // We can set the shader once, since we have only one
   



    int shad = shader;

    int vao = V;
    glUseProgram(shad);



    glBindVertexArray(vao);

    

    // We can set the shader once, since we have only one
    




    GLuint worldMatrixLocation = glGetUniformLocation(shad, "worldMatrix");

    GLuint decidedLocation = glGetUniformLocation(shad, "decided_color");

    
    glUniform1i(decidedLocation, 1);

    mat4 cube = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f)); //* scale(mat4(1.0f), vec3(1.0f, 1.0f, 2.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

   

    cube = translate(mat4(1.0f), vec3(0.0f, 1.0f, 0.0f)); //* scale(mat4(1.0f), vec3(1.0f, 1.0f, 2.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    
    cube = translate(mat4(1.0f), vec3(0.0f, 2.0f, 0.0f)); //* scale(mat4(1.0f), vec3(1.0f, 1.0f, 2.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);


    cube = translate(mat4(1.0f), vec3(1.0f, 0.0f, 0.0f)); //* scale(mat4(1.0f), vec3(1.0f, 1.0f, 2.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(0.0f, 0.0f, 1.0f)); //* scale(mat4(1.0f), vec3(1.0f, 1.0f, 2.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(0.0f, 0.0f, 2.0f)); //* scale(mat4(1.0f), vec3(1.0f, 1.0f, 2.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(0.0f, 0.0f, 3.0f)); //* scale(mat4(1.0f), vec3(1.0f, 1.0f, 2.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);


    cube = translate(mat4(1.0f), vec3(-1.0f, 0.0f, 0.0f)); //* scale(mat4(1.0f), vec3(1.0f, 1.0f, 2.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(-2.0f, 0.0f, 0.0f)); //* scale(mat4(1.0f), vec3(1.0f, 1.0f, 2.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);



    glUniform1i(decidedLocation, 0);
    cube = translate(mat4(1.0f), vec3(-2.0f, 4.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    //first row
    cube = translate(mat4(1.0f), vec3(-3.0f, 4.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(-4.0f, 4.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(-1.0f, 4.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(0.0f, 4.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(1.0f, 4.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(2.0f, 4.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    


     //coloum

    cube = translate(mat4(1.0f), vec3(-4.0f, 3.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(-4.0f, 2.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(-4.0f, 1.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(-4.0f, 0.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(-4.0f, -1.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(-4.0f, -2.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(-4.0f, -3.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);



    cube = translate(mat4(1.0f), vec3(-3.0f, 3.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(-3.0f, 2.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(-3.0f, 1.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(-3.0f, 0.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(-3.0f, -1.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(-3.0f, -2.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(-3.0f, -3.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);


    cube = translate(mat4(1.0f), vec3(-2.0f, 3.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(-2.0f, 2.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(-2.0f, 1.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(-2.0f, -1.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(-2.0f, -2.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(-2.0f, -3.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);


    cube = translate(mat4(1.0f), vec3(-1.0f, 3.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);


    cube = translate(mat4(1.0f), vec3(-1.0f, 2.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);


    cube = translate(mat4(1.0f), vec3(-1.0f, 1.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(-1.0f, -1.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(-1.0f, -2.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(-1.0f, -3.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);



    cube = translate(mat4(1.0f), vec3(0.0f, 3.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(0.0f, -1.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(0.0f, -2.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(0.0f, -3.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);



    cube = translate(mat4(1.0f), vec3(1.0f, 3.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(1.0f, 2.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(1.0f, 1.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(1.0f, -1.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(1.0f, -2.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(1.0f, -3.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);




    cube = translate(mat4(1.0f), vec3(2.0f, 3.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(2.0f, 2.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(2.0f, 1.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);



    cube = translate(mat4(1.0f), vec3(2.0f, -1.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    cube = translate(mat4(1.0f), vec3(2.0f, 0.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);


    cube = translate(mat4(1.0f), vec3(2.0f, -2.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);


    cube = translate(mat4(1.0f), vec3(2.0f, -2.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);


    cube = translate(mat4(1.0f), vec3(2.0f, -3.0f, -3.0f));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &cube[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);




}







bool initContext() {     // Initialize GLFW and OpenGL version
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
    window = glfwCreateWindow(800, 600, "Comp371 - Lab 03", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);

    // @TODO 3 - Disable mouse cursor
    // ...

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to create GLEW" << std::endl;
        glfwTerminate();
        return false;
    }
    return true;
}


