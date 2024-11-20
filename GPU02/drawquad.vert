#version 410 core

in vec3 in_Position;
out vec2 uv;

void main(void)
{
	uv = vec2(in_Position.xy);

	// map [0 ~ 1] to [-1 ~ 1]

	gl_Position = vec4(uv * 2 - 1, 0, 1);
}