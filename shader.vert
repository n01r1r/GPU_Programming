#version 410 core

layout(location=0) in vec3 in_Position;
layout(location=1) in vec3 in_Normal;

uniform vec3 cameraPosition;
uniform mat4 modelMat = mat4(1);
uniform mat4 viewMat = mat4(1);
uniform mat4 projMat = mat4(1);
uniform mat3 normalMat;

out VS_GS{
	out vec3 VNormal;
	out vec3 VPosition;
	out float ndv;
} vs_out;

void main(void)
{	
	vs_out.VPosition = vec3(modelMat * vec4(in_Position, 1.0)); 
	vs_out.VNormal = mat3(transpose(inverse(modelMat))) * in_Normal;

	gl_Position = projMat * viewMat * vec4(vs_out.VPosition, 1.0);

	vec3 viewDir = normalize(cameraPosition - vs_out.VPosition);
	vs_out.ndv = dot(vs_out.VNormal, viewDir);
	/*
	vs_out.VPosition = vec3(modelMat * vec4(in_Position, 1.0));  
	vs_out.VNormal = normalize(in_Normal);

	gl_Position = projMat * viewMat * vec4(vs_out.VPosition, 1.0);

	vec3 viewDir = normalize(cameraPosition - vs_out.VPosition); 
	vs_out.ndv = max(dot(vs_out.VNormal, viewDir), 0.0);
*/
}

