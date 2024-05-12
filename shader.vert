#version 400

layout(location=0) in vec3 in_Position;
layout(location=1) in vec3 in_Normal;

uniform vec3 cameraPosition;
uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projMat;
uniform mat3 normalMat;

out vec3 Vnorm;
out vec3 Vpos;
out float ndotv;

void main(void)
{	
	vec3 Vpos = vec3(viewMat * modelMat * vec4(in_Position, 1.0));  
	vec3 Vnorm = normalize(normalMat * in_Normal);

	gl_Position = projMat * viewMat * vec4(Vpos, 1.0);

	vec3 viewDir = normalize(cameraPosition - Vpos); 
	float nv = max(dot(Vnorm, viewDir), 0.0); 
	ndotv = nv;
}

