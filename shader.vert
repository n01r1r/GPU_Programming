#version 410 core

layout(location=0) in vec3 in_Position;
layout(location=1) in vec3 in_Normal;

uniform vec3 cameraPosition;
uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projMat;
uniform mat3 normalMat;

out vec3 VNormal;
out vec3 VPosition;
out float ndotv;

void main(void)
{	
	vec3 VPosition = vec3(modelMat * vec4(in_Position, 1.0));  
	vec3 VNormal = normalize(in_Normal);

	gl_Position = projMat * viewMat * vec4(VPosition, 1.0);

	vec3 viewDir = normalize(cameraPosition - VPosition); 
	float nv = max(dot(VNormal, viewDir), 0.0); 
	ndotv = nv;
}

