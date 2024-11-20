#version 410 core

layout(location=0) in vec3 in_Position;
layout(location=1) in vec3 in_Normal;
layout(location=2) in vec2 in_TexCoord;

uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projMat;

out vec3 pass_Normal;
out vec3 pass_WorldPos;
out vec2 pass_TexCoord;

void main(void)
{

	vec3 vertPos = in_Position;

	pass_WorldPos = (modelMat * vec4(vertPos, 1)).xyz;
	pass_Normal = (modelMat * vec4(in_Normal,0)).xyz;
	pass_TexCoord = in_TexCoord;

	vec4 p = projMat * viewMat * modelMat * vec4(vertPos, 1);
	gl_Position = p;
}

