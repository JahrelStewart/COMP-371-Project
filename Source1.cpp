#include <iostream>


//#define GLEW_STATIC 1   // This allows linking with Static Library on Windows, without DLL
#include <glew.h>    // Include GLEW - OpenGL Extension Wrangler
#include"GLFW/glfw3.h"


#include <glm/glm.hpp>  // GLM is an optimized math library with syntax to similar to OpenGL Shading Language
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#pragma comment(lib,"opengl32.lib")//for linking
#pragma comment(lib,"glew32.lib")//for linking
#pragma comment(lib, "glfw3.lib")//for linking
GLuint gShaderProgramObject;
float zValue = -65.0f;

enum
{
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_COLOR,
    AMC_ATTRIBUTE_NORMAL,
    AMC_ATTRIBUTE_TEXCOORD0
};


//
GLuint vao_triangle;
GLuint vao_rectangle;
GLuint vbo_position_triangle;
GLuint vbo_position_rectangle;
GLuint vbo_color_rectangle;
GLuint vbo_color_triangle;
GLuint mvpUniform;
glm::mat4 projection;


GLuint colorUniform;


int grid_size = 100;
int totalTiles = (int)(pow(grid_size, 2.0) + 0.5);

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
        glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3((float)170 / 255, (float)185 / 255, (float)207 / 255),
        glm::vec3(-0.5f, 0.5f, 0.0f), glm::vec3((float)170 / 255, (float)185 / 255, (float)207 / 255),
        glm::vec3(0.5f, 0.5f, 0.0f), glm::vec3((float)170 / 255, (float)185 / 255, (float)207 / 255),
        glm::vec3(0.5f, -0.5f, 0.0f), glm::vec3((float)170 / 255, (float)185 / 255, (float)207 / 255),
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
            quadPos.x = (float)(j - offset);
            quadPos.y = (float)(i - offset);
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
        (void*)0             // array buffer offset
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
        (void*)0
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

    glBindVertexArray(0); // Unbind to not modify the VAO

    return vertexArrayObject;
}
void resize(int width, int height)
{
	if (height == 0)
	{
		height = 1;
	}

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
}
void initShaders()
{
	//////////////////////////////// V E R T E X - S H A D E R //////////////////////////
	GLuint gVertexShaderObject;
	GLuint gFragmentShaderObject;

	//Define Vertex Shader Object
	gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER); //This command will create the Shader Object
	//Now Write vertex shader code
	const GLchar** p;
	const GLchar* vertexShaderSourceCode =

		"#version 430 core" \
		"\n"
		"layout(location  = 0)in vec4 vPosition;" \
		"layout(location = 1)in vec4 vColor;" \
		"out vec4 out_color;" \
		"uniform mat4 u_mvp_matrix;" \

		"void main(void)" \
		"{" \
		"gl_Position = u_mvp_matrix * vPosition;" \
		"out_color = vColor;" \
		"}";
	// GPU will run the above code. And GPU WILL RUN FOR PER VERTEX. If there are 1000 vertex. Then GPU will run this shader for
	//1000 times. We are Multiplying each vertex with the Model View Matrix.
	//And how does the GPU gets to know about at what offset the array has to be taken . Go to glVertexAttribPointer() in Display.
	// in = Input. 

	
		//Specify above source code to the vertex shader object
	glShaderSource(gVertexShaderObject, 1, (const GLchar**)&vertexShaderSourceCode, NULL);

	//Compile the vertex shader 
	glCompileShader(gVertexShaderObject);

	//////////////// Error Checking//////////////////
	//Code for catching the errors 
	GLint iShaderCompileStatus = 0;
	GLint iInfoLogLength = 0;
	GLchar* szInfoLog = NULL;


	glGetShaderiv(gVertexShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);
	if (iShaderCompileStatus == GL_FALSE)
	{
		glGetShaderiv(gVertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (GLchar*)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gVertexShaderObject, iInfoLogLength, &written, szInfoLog);
			
				free(szInfoLog);
			
				exit(0);


			}
		}
	}
	/////////////////    F R A G M E N T S H A D E R            //////////////////////////
	//Define Fragment Shader Object
	gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER); //This command will create the Shader Object
	//Now Write vertex shader code
	const GLchar* fragmentShaderSourceCode =
		"#version 430 core" \
		"\n" \
		"in vec4 out_color;" \
		"out vec4 FragColor;" \
		"uniform int decideColor;" \
		"void main(void)" \
		"{" \
		"if(decideColor == 0)" \
		"{" \
		"FragColor = vec4(1.0,0.5,0.0,1.0);" \
		"}" \
		"else" \
		"{" \
		"FragColor = vec4(1.0);" \
		"}" \
		"}";

	


	//Specify above source code to the vertex shader object
	glShaderSource(gFragmentShaderObject, 1, (const GLchar**)&fragmentShaderSourceCode, NULL);

	//Compile the vertex shader 
	glCompileShader(gFragmentShaderObject);
	//Code for catching the errors 
		  
	szInfoLog = NULL;


	glGetShaderiv(gFragmentShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);
	if (iShaderCompileStatus == GL_FALSE)
	{
		glGetShaderiv(gFragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (GLchar*)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written1;
				glGetShaderInfoLog(gFragmentShaderObject, iInfoLogLength, &written1, szInfoLog);
				
				free(szInfoLog);
			
				exit(0);


			}
		}
	}
	// CREATE SHADER PROGRAM OBJECT
	gShaderProgramObject = glCreateProgram();
	//attach vertex shader to the gShaderProgramObject
	glAttachShader(gShaderProgramObject, gVertexShaderObject);

	//attach fragment shader to the gShaderProgramObject
	glAttachShader(gShaderProgramObject, gFragmentShaderObject);



	//Link the shader program 
	glLinkProgram(gShaderProgramObject);

	//Code for catching the errors 
	GLint iProgramLinkStatus = 0;



	glGetProgramiv(gShaderProgramObject, GL_LINK_STATUS, &iProgramLinkStatus);
	if (iProgramLinkStatus == GL_FALSE)
	{
		glGetProgramiv(gShaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (GLchar*)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written3;
				glGetProgramInfoLog(gShaderProgramObject, iInfoLogLength, &written3, szInfoLog);
				
				free(szInfoLog);
				
				exit(0);


			}
		}
	}


	//POST Linking
	//Retrieving uniform locations 
	mvpUniform = glGetUniformLocation(gShaderProgramObject, "u_mvp_matrix");
	colorUniform = glGetUniformLocation(gShaderProgramObject, "decideColor");
	//Here we have done all the preparations of data transfer from CPU to GPU

	
	const GLfloat rectangleVertices[] =
	{
		// Perspective square (Top face)
		1.0f, 1.0f, -1.0f,		// Right top
		-1.0f, 1.0f, -1.0f, 	// Left top
		-1.0f, 1.0f, 1.0f,		// Left bottom
		1.0f, 1.0f, 1.0f,		// Right bottom
								// Perspective square (Bottom face)
		1.0f, -1.0f, -1.0f,		// Right top
		-1.0f, -1.0f, -1.0f, 	// Left top
		-1.0f, -1.0f, 1.0f,		// Left bottom
		1.0f, -1.0f, 1.0f,		// Right bottom
								// Perspective square (Front face)
		1.0f, 1.0f, 1.0f,		// Right top
		-1.0f, 1.0f, 1.0f,		// Left top
		-1.0f, -1.0f, 1.0f, 	// Left bottom
		1.0f, -1.0f, 1.0f,		// Right bottom
								// Perspective square (Back face)
		1.0f, 1.0f, -1.0f,		// Right top											
		-1.0f, 1.0f, -1.0f,		// Left top
		-1.0f, -1.0f, -1.0f, 	// Left bottom
		1.0f, -1.0f, -1.0f,		// Right bottom
								// Perspective square (Right face)
		1.0f, 1.0f, -1.0f,		// Right top											
		1.0f, 1.0f, 1.0f,		// Left top
		1.0f, -1.0f, 1.0f, 		// Left bottom
		1.0f, -1.0f, -1.0f,		// Right bottom
								// Perspective square (Left face)
		-1.0f, 1.0f, 1.0f,		// Right top																						
		-1.0f, 1.0f, -1.0f,		// Left top
		-1.0f, -1.0f, -1.0f, 	// Left bottom
		-1.0f, -1.0f, 1.0f		// Right bottom


	};
	const GLfloat rectangleColor[] =
	{
		0.0f, 0.0f, 1.0f,		// Red 		- Top face
		0.0f, 0.0f, 1.0f,		// Red
		0.0f, 0.0f, 1.0f,		// Red
		0.0f, 0.0f, 1.0f,		// Red
		0.0f, 0.0f, 1.0f,		// Green 	- Bottom face
		0.0f, 0.0f, 1.0f,		// Green
		0.0f, 0.0f, 1.0f,		// Green											
		0.0f, 0.0f, 1.0f,		// Green
		0.0f, 0.0f, 1.0f,		// Blue		- Front face
		0.0f, 0.0f, 1.0f,		// Blue
		0.0f, 0.0f, 1.0f,		// Blue
		0.0f, 0.0f, 1.0f,		// Blue
		0.0f, 1.0f, 1.0f,		// Cyan		- Back face
		0.0f, 1.0f, 1.0f,		// Cyan
		0.0f, 1.0f, 1.0f,		// Cyan
		0.0f, 1.0f, 1.0f,		// Cyan
		0.0f, 0.0f, 1.0f,		// Magenta 	- Right face
		0.0f, 0.0f, 1.0f,		// Magenta
		0.0f, 0.0f, 1.0f,		// Magenta
		0.0f, 0.0f, 1.0f,		// Magenta
		0.0f, 0.0f, 1.0f,		// Yellow	- Left face
		0.0f, 0.0f, 1.0f,		// Yellow
		0.0f, 0.0f, 1.0f,		// Yellow											
		0.0f, 0.0f, 1.0f };
	

	//////////////////////// FOR RECTANGLE //////////////// 

	glGenVertexArrays(1, &vao_rectangle);
	glBindVertexArray(vao_rectangle);
	glGenBuffers(1, &vbo_position_rectangle);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_position_rectangle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), rectangleVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_color_rectangle);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_color_rectangle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleColor), rectangleColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);


	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	projection = glm::mat4(1.0);
	resize(1024, 768);
}
void drawShape()
{

	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// use shader program
	glUseProgram(gShaderProgramObject);

	//declaration of matrices
	glm::mat4 modelViewMatrix;
	glm::mat4 modelViewProjectionMatrix;
	glm::mat4 RotationMatrix;
	glm::mat4 TranslateMatrix;
	glm::mat4 scaleMatrix;
	glm::mat4 viewMatrix;

	// intialize above matrices to identity
	viewMatrix = glm::mat4(1.0);
	viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 15.0f), glm::vec3(0.0f, 0.0f, -5.0f), glm::vec3(0.0f, 1.0f, 0.0f));


	modelViewMatrix = glm::mat4(1.0);
	modelViewProjectionMatrix = glm::mat4(1.0);
	RotationMatrix = glm::mat4(1.0);
	TranslateMatrix = glm::mat4(1.0);
	TranslateMatrix = glm::translate(TranslateMatrix, glm::vec3(-2.5f, 0.0f, -9.0f));
	//
	// perform necessary transformations
	modelViewMatrix = viewMatrix * TranslateMatrix * RotationMatrix;

	modelViewProjectionMatrix = projection * modelViewMatrix;

	// send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, glm::value_ptr(modelViewProjectionMatrix));
	glUniform1i(colorUniform, 1);
	



	modelViewMatrix = glm::mat4(1.0);
	modelViewProjectionMatrix = glm::mat4(1.0);
	RotationMatrix = glm::mat4(1.0);
	TranslateMatrix = glm::mat4(1.0);
	scaleMatrix = glm::mat4(1.0);


	TranslateMatrix = translate(TranslateMatrix, glm::vec3(0.0f, 21.0f, -70.0f));
	//RotationMatrix = rotate(angleRectangle, angleRectangle, angleRectangle);

	scaleMatrix = glm::scale(scaleMatrix, glm::vec3(25.3f, 10.2f, 10.2f));
	// perform necessary transformations
	modelViewMatrix = viewMatrix * TranslateMatrix * scaleMatrix;
	modelViewProjectionMatrix = projection * modelViewMatrix;

	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, glm::value_ptr(modelViewProjectionMatrix));
	glBindVertexArray(vao_rectangle);

	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);

	glBindVertexArray(0);







	//UPPER MIDDLE QUAD

	modelViewMatrix = glm::mat4(1.0);
	modelViewProjectionMatrix = glm::mat4(1.0);
	RotationMatrix = glm::mat4(1.0);
	TranslateMatrix = glm::mat4(1.0);
	scaleMatrix = glm::mat4(1.0);


	TranslateMatrix = translate(TranslateMatrix, glm::vec3(0.0f, 16.5f, -70.0f));
	//RotationMatrix = rotate(angleRectangle, angleRectangle, angleRectangle);

	scaleMatrix = glm::scale(scaleMatrix, glm::vec3(0.3f, 10.2f, 10.2f));
	// perform necessary transformations
	modelViewMatrix = viewMatrix * TranslateMatrix * scaleMatrix;
	modelViewProjectionMatrix = projection * modelViewMatrix;

	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, glm::value_ptr(modelViewProjectionMatrix));
	glBindVertexArray(vao_rectangle);

	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);

	glBindVertexArray(0);

	/// Left hand side Quad Part 1 
	modelViewMatrix = glm::mat4(1.0);
	modelViewProjectionMatrix = glm::mat4(1.0);
	RotationMatrix = glm::mat4(1.0);
	TranslateMatrix = glm::mat4(1.0);
	scaleMatrix = glm::mat4(1.0);


	TranslateMatrix = translate(TranslateMatrix, glm::vec3(-15.4f, 10.0f, -70.0f));
	

	scaleMatrix = glm::scale(scaleMatrix, glm::vec3(9.9f, 3.6f, 10.2f));
	// perform necessary transformations
	modelViewMatrix = viewMatrix * TranslateMatrix * scaleMatrix;
	modelViewProjectionMatrix = projection * modelViewMatrix;

	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, glm::value_ptr(modelViewProjectionMatrix));
	glBindVertexArray(vao_rectangle);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
	glBindVertexArray(0);


	/// Left hand side Quad Part 2
	modelViewMatrix = glm::mat4(1.0);
	modelViewProjectionMatrix = glm::mat4(1.0);
	RotationMatrix = glm::mat4(1.0);
	TranslateMatrix = glm::mat4(1.0);
	scaleMatrix = glm::mat4(1.0);


	TranslateMatrix = translate(TranslateMatrix, glm::vec3(-15.8f, 4.6f, -70.0f));
	

	scaleMatrix = glm::scale(scaleMatrix, glm::vec3(9.5f, 1.9f, 10.2f));
	// perform necessary transformations
	modelViewMatrix = viewMatrix * TranslateMatrix * scaleMatrix;
	modelViewProjectionMatrix = projection * modelViewMatrix;

	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, glm::value_ptr(modelViewProjectionMatrix));
	glBindVertexArray(vao_rectangle);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
	glBindVertexArray(0);


	/// Left hand side Quad Part 3
	modelViewMatrix = glm::mat4(1.0);
	modelViewProjectionMatrix = glm::mat4(1.0);
	RotationMatrix = glm::mat4(1.0);
	TranslateMatrix = glm::mat4(1.0);
	scaleMatrix = glm::mat4(1.0);


	TranslateMatrix = translate(TranslateMatrix, glm::vec3(-17.1f, -10.0f, -70.0f));
	

	scaleMatrix = glm::scale(scaleMatrix, glm::vec3(8.2f, 13.9f, 10.2f));
	// perform necessary transformations
	modelViewMatrix = viewMatrix * TranslateMatrix * scaleMatrix;
	modelViewProjectionMatrix = projection * modelViewMatrix;

	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, glm::value_ptr(modelViewProjectionMatrix));
	glBindVertexArray(vao_rectangle);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
	glBindVertexArray(0);


	/// Left hand side Quad Part 4
	modelViewMatrix = glm::mat4(1.0);
	modelViewProjectionMatrix = glm::mat4(1.0);
	RotationMatrix = glm::mat4(1.0);
	TranslateMatrix = glm::mat4(1.0);
	scaleMatrix = glm::mat4(1.0);


	TranslateMatrix = translate(TranslateMatrix, glm::vec3(-14.6f, -13.3f, -70.0f));
	
	scaleMatrix = glm::scale(scaleMatrix, glm::vec3(8.2f, 10.6f, 10.2f));
	// perform necessary transformations
	modelViewMatrix = viewMatrix * TranslateMatrix * scaleMatrix;
	modelViewProjectionMatrix = projection * modelViewMatrix;

	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, glm::value_ptr(modelViewProjectionMatrix));
	glBindVertexArray(vao_rectangle);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
	glBindVertexArray(0);


	/// Left hand side Quad Part 5 (Upper Quad)
	modelViewMatrix = glm::mat4(1.0);
	modelViewProjectionMatrix = glm::mat4(1.0);
	RotationMatrix = glm::mat4(1.0);
	TranslateMatrix = glm::mat4(1.0);
	scaleMatrix = glm::mat4(1.0);


	TranslateMatrix = translate(TranslateMatrix, glm::vec3(-13.6f, 12.75f, -70.0f));
	

	scaleMatrix = glm::scale(scaleMatrix, glm::vec3(9.9f, 3.6f, 10.2f));
	// perform necessary transformations
	modelViewMatrix = viewMatrix * TranslateMatrix * scaleMatrix;
	modelViewProjectionMatrix = projection * modelViewMatrix;

	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, glm::value_ptr(modelViewProjectionMatrix));
	glBindVertexArray(vao_rectangle);
	
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);

	glBindVertexArray(0);

	/***** Right Hand Side Quad****/


	/// Right hand side Quad Part 1 
	modelViewMatrix = glm::mat4(1.0);
	modelViewProjectionMatrix = glm::mat4(1.0);
	RotationMatrix = glm::mat4(1.0);
	TranslateMatrix = glm::mat4(1.0);
	scaleMatrix = glm::mat4(1.0);


	TranslateMatrix = translate(TranslateMatrix, glm::vec3(15.4f, 10.0f, -70.0f));
	
	scaleMatrix = glm::scale(scaleMatrix, glm::vec3(9.9f, 3.6f, 10.2f));
	// perform necessary transformations
	modelViewMatrix = viewMatrix * TranslateMatrix * scaleMatrix;
	modelViewProjectionMatrix = projection * modelViewMatrix;

	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, glm::value_ptr(modelViewProjectionMatrix));
	glBindVertexArray(vao_rectangle);
	
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
	
	glBindVertexArray(0);


	/// Right hand side Quad Part 2
	modelViewMatrix = glm::mat4(1.0);
	modelViewProjectionMatrix = glm::mat4(1.0);
	RotationMatrix = glm::mat4(1.0);
	TranslateMatrix = glm::mat4(1.0);
	scaleMatrix = glm::mat4(1.0);


	TranslateMatrix = translate(TranslateMatrix, glm::vec3(15.8f, 4.6f, -70.0f));
	

	scaleMatrix = glm::scale(scaleMatrix, glm::vec3(9.5f, 1.9f, 10.2f));
	// perform necessary transformations
	modelViewMatrix = viewMatrix * TranslateMatrix * scaleMatrix;
	modelViewProjectionMatrix = projection * modelViewMatrix;

	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, glm::value_ptr(modelViewProjectionMatrix));
	glBindVertexArray(vao_rectangle);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
	glBindVertexArray(0);


	/// Right hand side Quad Part 3
	modelViewMatrix = glm::mat4(1.0);
	modelViewProjectionMatrix = glm::mat4(1.0);
	RotationMatrix = glm::mat4(1.0);
	TranslateMatrix = glm::mat4(1.0);
	scaleMatrix = glm::mat4(1.0);


	TranslateMatrix = translate(TranslateMatrix, glm::vec3(17.1f, -10.0f, -70.0f));
	
	scaleMatrix = glm::scale(scaleMatrix, glm::vec3(8.2f, 13.9f, 10.2f));
	// perform necessary transformations
	modelViewMatrix = viewMatrix * TranslateMatrix * scaleMatrix;
	modelViewProjectionMatrix = projection * modelViewMatrix;

	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, glm::value_ptr(modelViewProjectionMatrix));
	glBindVertexArray(vao_rectangle);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
	glBindVertexArray(0);


	/// Right hand side Quad Part 4
	modelViewMatrix = glm::mat4(1.0);
	modelViewProjectionMatrix = glm::mat4(1.0);
	RotationMatrix = glm::mat4(1.0);
	TranslateMatrix = glm::mat4(1.0);
	scaleMatrix = glm::mat4(1.0);


	TranslateMatrix = translate(TranslateMatrix, glm::vec3(14.6f, -13.3f, -70.0f));
	
	scaleMatrix = glm::scale(scaleMatrix, glm::vec3(8.2f, 10.6f, 10.2f));
	// perform necessary transformations
	modelViewMatrix = viewMatrix * TranslateMatrix * scaleMatrix;
	modelViewProjectionMatrix = projection * modelViewMatrix;

	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, glm::value_ptr(modelViewProjectionMatrix));
	glBindVertexArray(vao_rectangle);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
	glBindVertexArray(0);


	/// Right hand side Quad Part 5 (Upper Quad)
	modelViewMatrix = glm::mat4(1.0);
	modelViewProjectionMatrix = glm::mat4(1.0);
	RotationMatrix = glm::mat4(1.0);
	TranslateMatrix = glm::mat4(1.0);
	scaleMatrix = glm::mat4(1.0);


	TranslateMatrix = translate(TranslateMatrix, glm::vec3(13.6f, 12.75f, -70.0f));
	
	scaleMatrix = glm::scale(scaleMatrix, glm::vec3(9.9f, 3.6f, 10.2f));
	// perform necessary transformations
	modelViewMatrix = viewMatrix * TranslateMatrix * scaleMatrix;
	modelViewProjectionMatrix = projection * modelViewMatrix;

	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, glm::value_ptr(modelViewProjectionMatrix));
	glBindVertexArray(vao_rectangle);
	
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
	
	glBindVertexArray(0);

	//Downward Quad
	modelViewMatrix = glm::mat4(1.0);
	modelViewProjectionMatrix = glm::mat4(1.0);
	RotationMatrix = glm::mat4(1.0);
	TranslateMatrix = glm::mat4(1.0);
	scaleMatrix = glm::mat4(1.0);


	TranslateMatrix = translate(TranslateMatrix, glm::vec3(0.0f, -16.6f, -70.0f));
	//RotationMatrix = rotate(angleRectangle, angleRectangle, angleRectangle);

	scaleMatrix = glm::scale(scaleMatrix, glm::vec3(25.3f, 10.2f, 10.2f));
	// perform necessary transformations
	modelViewMatrix = viewMatrix * TranslateMatrix * scaleMatrix;
	modelViewProjectionMatrix = projection * modelViewMatrix;

	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, glm::value_ptr(modelViewProjectionMatrix));
	glBindVertexArray(vao_rectangle);
	
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
	
	glBindVertexArray(0);







	glUniform1i(colorUniform, 0);

	/// Rendering the cube 
	modelViewMatrix = glm::mat4(1.0);
	modelViewProjectionMatrix = glm::mat4(1.0);
	RotationMatrix = glm::mat4(1.0);
	TranslateMatrix = glm::mat4(1.0);
	scaleMatrix = glm::mat4(1.0);


	TranslateMatrix = translate(TranslateMatrix, glm::vec3(-0.0f, 0.0f, zValue));
	

	scaleMatrix = glm::scale(scaleMatrix, glm::vec3(5.5f, 5.7f, 1.0f));
	// perform necessary transformations
	modelViewMatrix = viewMatrix * TranslateMatrix * scaleMatrix;
	modelViewProjectionMatrix = projection * modelViewMatrix;

	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, glm::value_ptr(modelViewProjectionMatrix));
	glBindVertexArray(vao_rectangle);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 4);
	glBindVertexArray(0);



	//2nd Upward Cube
	modelViewMatrix = glm::mat4(1.0);
	modelViewProjectionMatrix = glm::mat4(1.0);
	RotationMatrix = glm::mat4(1.0);
	TranslateMatrix = glm::mat4(1.0);
	scaleMatrix = glm::mat4(1.0);


	TranslateMatrix = translate(TranslateMatrix, glm::vec3(-2.7f, 6.5f, zValue));
	
	scaleMatrix = glm::scale(scaleMatrix, glm::vec3(2.4f, 1.65f, 1.0f));
	// perform necessary transformations
	modelViewMatrix = viewMatrix * TranslateMatrix * scaleMatrix;
	modelViewProjectionMatrix = projection * modelViewMatrix;

	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, glm::value_ptr(modelViewProjectionMatrix));
	glBindVertexArray(vao_rectangle);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 4);
	glBindVertexArray(0);



	//3rd Upward Cube
	modelViewMatrix = glm::mat4(1.0);
	modelViewProjectionMatrix = glm::mat4(1.0);
	RotationMatrix = glm::mat4(1.0);
	TranslateMatrix = glm::mat4(1.0);
	scaleMatrix = glm::mat4(1.0);


	TranslateMatrix = translate(TranslateMatrix, glm::vec3(2.7f, 6.5f, zValue));
	

	// perform necessary transformations
	scaleMatrix = glm::scale(scaleMatrix, glm::vec3(2.4f, 1.65f, 1.0f));
	modelViewMatrix = viewMatrix * TranslateMatrix * scaleMatrix;
	modelViewProjectionMatrix = projection * modelViewMatrix;

	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, glm::value_ptr(modelViewProjectionMatrix));
	glBindVertexArray(vao_rectangle);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 4);
	glBindVertexArray(0);


	//Right hand side cube
	//
	modelViewMatrix = glm::mat4(1.0);
	modelViewProjectionMatrix = glm::mat4(1.0);
	RotationMatrix = glm::mat4(1.0);
	TranslateMatrix = glm::mat4(1.0);
	scaleMatrix = glm::mat4(1.0);


	TranslateMatrix = translate(TranslateMatrix, glm::vec3(5.3f, 0.0f, zValue));
	
	scaleMatrix = glm::scale(scaleMatrix, glm::vec3(2.4f, 2.3f, 1.0f));
	// perform necessary transformations
	modelViewMatrix = viewMatrix * TranslateMatrix * scaleMatrix;
	modelViewProjectionMatrix = projection * modelViewMatrix;

	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, glm::value_ptr(modelViewProjectionMatrix));
	glBindVertexArray(vao_rectangle);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 4);
	glBindVertexArray(0);


	//Left 
	//Right hand side cube
	//
	modelViewMatrix = glm::mat4(1.0);
	modelViewProjectionMatrix = glm::mat4(1.0);
	RotationMatrix = glm::mat4(1.0);
	TranslateMatrix = glm::mat4(1.0);
	scaleMatrix = glm::mat4(1.0);


	TranslateMatrix = translate(TranslateMatrix, glm::vec3(-5.3f, 0.0f, zValue));
	//RotationMatrix = rotate(angleRectangle, angleRectangle, angleRectangle);

	scaleMatrix = glm::scale(scaleMatrix, glm::vec3(2.4f, 2.3f, 1.0f));
	// perform necessary transformations
	modelViewMatrix = viewMatrix * TranslateMatrix * scaleMatrix;
	modelViewProjectionMatrix = projection * modelViewMatrix;

	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, glm::value_ptr(modelViewProjectionMatrix));
	glBindVertexArray(vao_rectangle);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 4);
	glBindVertexArray(0);

	//Left hand side Antenna
	//Left 

	modelViewMatrix = glm::mat4(1.0);
	modelViewProjectionMatrix = glm::mat4(1.0);
	RotationMatrix = glm::mat4(1.0);
	TranslateMatrix = glm::mat4(1.0);
	scaleMatrix = glm::mat4(1.0);


	TranslateMatrix = translate(TranslateMatrix, glm::vec3(-1.8f, 7.0f, zValue));
	//RotationMatrix = rotate(angleRectangle, angleRectangle, angleRectangle);


	scaleMatrix = glm::scale(scaleMatrix, glm::vec3(1.4f, 2.5f, 1.0f));
	// perform necessary transformations
	modelViewMatrix = viewMatrix * TranslateMatrix * scaleMatrix;
	modelViewProjectionMatrix = projection * modelViewMatrix;

	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, glm::value_ptr(modelViewProjectionMatrix));
	glBindVertexArray(vao_rectangle);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 4);
	glBindVertexArray(0);

	//Right hand side Antenna
	//Right

	modelViewMatrix = glm::mat4(1.0);
	modelViewProjectionMatrix = glm::mat4(1.0);
	RotationMatrix = glm::mat4(1.0);
	TranslateMatrix = glm::mat4(1.0);
	scaleMatrix = glm::mat4(1.0);


	TranslateMatrix = translate(TranslateMatrix, glm::vec3(1.8f, 7.0f, zValue));
	//RotationMatrix = rotate(angleRectangle, angleRectangle, angleRectangle);

	scaleMatrix = glm::scale(scaleMatrix, glm::vec3(1.4f, 2.5f, 1.2f));
	// perform necessary transformations
	modelViewMatrix = viewMatrix * TranslateMatrix * scaleMatrix;
	modelViewProjectionMatrix = projection * modelViewMatrix;

	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, glm::value_ptr(modelViewProjectionMatrix));
	glBindVertexArray(vao_rectangle);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 4);
	glBindVertexArray(0);

	
	// unuse program
	//glUseProgram(0);
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

	
    //// Smokey Dark Blue background
    //glClearColor((float)13 / 255, (float)19 / 255, (float)33 / 255, 1.0f);

    //// Compile and link shaders here ...
    //int shaderProgram = compileAndLinkShaders(*getVertexShaderSource(), *getFragmentShaderSource());

    //// Define and upload geometry to the GPU here ...
    //int vao = createVertexArrayObject();

    ////calculate model matrix..which is the transform of the object we want to draw 
    //glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); //Rotate grid 90 degrees so that it lays flat on the XZ plane

    ////calculate view matrix..which is essentially the view of the camera         
    //glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, -80.0f));
    //view = glm::rotate(view, glm::radians(20.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    //create projection matrix...which is like mapping the coordinates of the current space to -1 to 1 space
   

   // glUseProgram(shaderProgram);
    //int model_location = glGetUniformLocation(shaderProgram, "u_Model_View_Projection");
    //glm::mat4 transformation_result = projection * view * model;
    //glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(transformation_result));

	//glUseProgram(0);
	initShaders();
	
	 projection = glm::perspective(45.0f, (float)1024 / (float)768, 0.1f, 1000.0f);
    // std::cout << glGetString(GL_VERSION);

    // Entering Main Loop
    while (!glfwWindowShouldClose(window)) {
        // Each frame, reset color of each pixel to glClearColor
        glClear(GL_COLOR_BUFFER_BIT);

        //glBindVertexArray(vao);
        //glDrawArraysInstanced(GL_LINE_LOOP, 0, 4, totalTiles);
        //// glDrawArrays(GL_LINES, 0, 6);
        //// glLineWidth(5.0f);

        //glBindVertexArray(0);
		drawShape();
        // End frame
        glfwSwapBuffers(window);

        // Detect inputs
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			zValue -= 1.0f;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			zValue += 1.0f;
    }

    // Shutdown GLFW
    glfwTerminate();

    return 0;
}
