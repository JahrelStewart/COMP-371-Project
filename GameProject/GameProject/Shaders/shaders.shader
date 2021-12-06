#shader gridShader
#version 330 core

layout(location = 0) in vec3 aGridPos;
layout(location = 1) in vec3 aGridColor;
layout(location = 2) in vec2 aGridTexture;
layout(location = 3) in vec3 gridNormals;
layout(location = 4) in vec3 gridOffsetPos;

out vec3 vertexColor;
out vec2 TexCoord;
out vec3 Normal;
out vec3 objPos;

uniform mat4 u_Model_View_Projection;
uniform mat4 u_Model;

void main() {
	vertexColor = aGridColor;
	TexCoord = aGridTexture;
	Normal = mat3(transpose(inverse(u_Model))) * gridNormals;
	objPos = vec3(u_Model * vec4(aGridPos + gridOffsetPos, 1.0));
	gl_Position = u_Model_View_Projection * vec4(aGridPos + gridOffsetPos, 1.0);
}

#shader wallShader
#version 330 core
layout(location = 5) in vec3 aWallPos;
layout(location = 6) in vec2 wallTextureCoord;
layout(location = 7) in vec3 wallNormals;
layout(location = 8) in vec3 wallOffsetPos;

out vec2 TexCoord;
out vec3 Normal;
out vec3 objPos;

uniform mat4 u_Model_View_Projection;
uniform mat4 u_Model;

void main() {
	TexCoord = wallTextureCoord;
	Normal = mat3(transpose(inverse(u_Model))) * wallNormals;
	objPos = vec3(u_Model * vec4(aWallPos + wallOffsetPos, 1.0));
	gl_Position = u_Model_View_Projection * vec4(aWallPos + wallOffsetPos, 1.0);
}

#shader cubeShader
#version 330 core

layout(location = 9) in vec3 aCubePos;
layout(location = 10) in vec2 cubeTexCoord;
layout(location = 11) in vec3 cubeNormals;

out vec2 TexCoord;
out vec3 Normal;
out vec3 objPos;
uniform bool isEnclosingCube;

uniform mat4 u_Model_View_Projection;
uniform mat4 u_Model;

void main() {
	vec3 norm = cubeNormals;
	if (isEnclosingCube) {
		norm = cubeNormals * -1.0;
	}

	TexCoord = cubeTexCoord;
	Normal = mat3(transpose(inverse(u_Model))) * norm;
	objPos = vec3(u_Model * vec4(aCubePos, 1.0));
	gl_Position = u_Model_View_Projection * vec4(aCubePos, 1.0);
}

#shader shadowMapShader
#version 330 core

layout(location = 0) in vec3 aGridPos;
layout(location = 4) in vec3 gridOffsetPos;

layout(location = 5) in vec3 aWallPos;
layout(location = 8) in vec3 wallOffsetPos;

layout(location = 9) in vec3 aCubePos;

uniform mat4 u_Model;
uniform bool is_Grid;
uniform bool is_Wall;

void main() {
	if (is_Grid) {
		gl_Position = u_Model * vec4(aGridPos + gridOffsetPos, 1.0);
	}
	else if (is_Wall) {
		gl_Position = u_Model * vec4(aWallPos + wallOffsetPos, 1.0);
	}
	else {
		gl_Position = u_Model * vec4(aCubePos, 1.0);
	}
}

#shader geometryShader
#version 330 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

uniform mat4 shadowMatrices[6];

out vec4 FragPos;

void main() {
	for (int face = 0; face < 6; ++face) {
		gl_Layer = face;
		for (int i = 0; i < 3; ++i) {
			FragPos = gl_in[i].gl_Position;
			gl_Position = shadowMatrices[face] * FragPos;
			EmitVertex();
		}
		EndPrimitive();
	}
}

#shader shadowFragmentShader
#version 330 core

in vec4 FragPos;

uniform vec3 lightPos;
uniform float far_plane;

void main() {
	float lightDistance = length(FragPos.xyz - lightPos);
	// map to [0;1] range by dividing by far_plane
	lightDistance = lightDistance / far_plane;

	gl_FragDepth = lightDistance;
}

#shader fragmentLightShadowTextureShader
#version 330 core

in vec2 TexCoord;
in vec3 Normal;
in vec3 objPos;
in vec3 vertexColor;

out vec4 FragColor;

uniform sampler2D customTexture;
uniform sampler2D texture_diffuse1;
uniform samplerCube shadowMap;

uniform bool textureToggle;
uniform bool isCube;
uniform bool isLightSource;
uniform bool isModel;
uniform vec3 u_Color;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform float far_plane;
uniform bool shadows;

float ShadowCalculation(vec3 fragPos) {

	// get vector between fragment position and light position
	vec3 fragToLight = fragPos - lightPos;
	// ise the fragment to light vector to sample from the depth map    
	float closestDepth = texture(shadowMap, fragToLight).r;
	// it is currently in linear range between [0,1], let's re-transform it back to original depth value
	closestDepth *= far_plane;
	// now get current linear depth as the length between the fragment and light position
	float currentDepth = length(fragToLight);
	// test for shadows
	float bias = 2.0; // we use a much larger bias since depth is now in [near_plane, far_plane] range
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

	return shadow;
}

void main() {

	if (isLightSource) {
		FragColor = vec4(u_Color, 1.0);
	}
	else {
		vec3 objectColor;

		if (isCube) {
			objectColor = u_Color;
		}
		else {
			objectColor = vertexColor;
		}

		vec3 color = textureToggle ? texture(customTexture, TexCoord).rgb : lightColor;

		// ambient
		float ambientStrength = 4.0;
		vec3 ambient = ambientStrength * color;

		// diffuse 
		float diffuseStrength = 5.0;
		vec3 norm = normalize(Normal);
		vec3 lightDir = normalize(lightPos - objPos);
		float diff = max(dot(norm, lightDir), 0.0);
		vec3 diffuse = diffuseStrength * diff * color;

		// specular
		float specularStrength = 6.0;
		vec3 viewDir = normalize(viewPos - objPos);
		vec3 reflectDir = reflect(-lightDir, norm);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), 75);
		vec3 specular = specularStrength * spec * color;

		// attenuation: Creates point light effect
		float lightConstant = 1.0f;
		float lightLinear = 0.009f;
		float lightQuadratic = 0.0032f;
		float distance = length(lightPos - objPos);
		float attenuation = 1.0f / (lightConstant + lightLinear * distance + lightQuadratic * (distance * distance));

		ambient *= attenuation;
		diffuse *= attenuation;
		specular *= attenuation;

		// calculate shadow
		float shadow = shadows ? ShadowCalculation(objPos) : 0.0;

		vec3 lighting = textureToggle ? (ambient + (1.0 - shadow) * (diffuse + specular)) * lightColor : (ambient + (1.0 - shadow) * (diffuse + specular)) * objectColor;

		FragColor = vec4(lighting, 1.0);

	}

}

#shader textVertex
#version 330 core
layout(location = 12) in vec4 text;
out vec2 TexCoords;

uniform mat4 textProjection;

void main() {
	gl_Position = textProjection * vec4(text.xy, 0.0, 1.0);
	TexCoords = text.zw;
}

#shader textFragment
#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D textTexture;
uniform vec3 textColor;

void main() {
	vec4 sampled = vec4(1.0, 1.0, 1.0, texture(textTexture, TexCoords).r);
	color = vec4(textColor, 1.0) * sampled;
}

#shader modelVertex
#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out vec2 TexCoord;
out vec3 Normal;
out vec3 objPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
	TexCoord = aTexCoords;
	Normal = mat3(transpose(inverse(model))) * aNormal;
	objPos = vec3(model * vec4(aPos, 1.0));
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}

#shader modelFragment
#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 objPos;

uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform sampler2D texture_diffuse1;
uniform vec3 u_Color;

void main() {
	// * vec4(u_color, 1.0)
	vec3 color = lightColor;
	vec3 objectColor = u_Color;

	// ambient
	float ambientStrength = 4.0;
	vec3 ambient = ambientStrength * color;

	// diffuse 
	float diffuseStrength = 5.0;
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - objPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diffuseStrength * diff * color;

	// specular
	float specularStrength = 6.0;
	vec3 viewDir = normalize(viewPos - objPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 75);
	vec3 specular = specularStrength * spec * color;

	// attenuation: Creates point light effect
	float lightConstant = 1.0f;
	float lightLinear = 0.009f;
	float lightQuadratic = 0.0032f;
	float distance = length(lightPos - objPos);
	float attenuation = 1.0f / (lightConstant + lightLinear * distance + lightQuadratic * (distance * distance));

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;		

	vec3 lighting = (ambient + (1.0 - 0.0) * (diffuse + specular)) * objectColor;

	FragColor = vec4(lighting, 1.0);
}
