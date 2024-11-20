#version 410 core

layout(location=0) in vec3 in_Position;
layout(location=1) in vec3 in_Normal;

uniform vec3 cameraPosition;
uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projMat;
uniform mat3 normalMat;

out vec3 wnorm;
out vec3 wpos;
out float ndv;
	

void main(void)
{	
	wpos = vec3(modelMat * vec4(in_Position, 1.0));  
	wnorm = normalize(in_Normal);

	gl_Position = projMat * viewMat * vec4(wpos, 1.0);

	vec3 viewDir = normalize(cameraPosition - wpos); 
	ndv = max(dot(wnorm, viewDir), 0.0); 
}

