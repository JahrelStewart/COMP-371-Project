#shader gridShader
#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec3 offsetPos;
out vec3 vertexColor;

uniform mat4 u_Model_View_Projection;

void main()
{
    vertexColor = aColor;
    gl_Position = u_Model_View_Projection * vec4(aPos + offsetPos, 1.0);
}

#shader threeLineShader
#version 330 core

layout(location = 3) in vec3 aLinePos;
layout(location = 4) in vec3 aLineColor;
out vec3 vertexColor;

uniform mat4 u_Model_View_Projection;

void main()
{
    vertexColor = aLineColor;
    gl_Position = u_Model_View_Projection * vec4(aLinePos, 1.0);
}

#shader wallShader
#version 330 core

layout(location = 5) in vec3 aWallPos;
layout(location = 6) in vec3 wallOffsetPos;

uniform mat4 u_Model_View_Projection;

void main()
{
    gl_Position = u_Model_View_Projection * vec4(aWallPos + wallOffsetPos, 1.0);
}

#shader cubeShader
#version 330 core

layout(location = 7) in vec3 aCubePos;

uniform mat4 u_Model_View_Projection;

void main()
{
    gl_Position = u_Model_View_Projection * vec4(aCubePos, 1.0);
}

#shader fragmentShader
#version 330 core

in vec3 vertexColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(vertexColor.r, vertexColor.g, vertexColor.b, 1.0f);
}

#shader cubeFragmentShader
#version 330 core

in vec3 vertexColor;
out vec4 FragColor;

uniform vec3 u_Color;

void main()
{
    FragColor = vec4(u_Color, 1.0f);
}