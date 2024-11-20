#version 410 core

in GS_FS{
	in vec3 GNormal;
	in vec3 GPosition;
} gs_out;

uniform int	pass;
uniform vec3 lineColor;

flat in int GIsEdge;
out vec4 FragColor;

void main() {
    if(GIsEdge==1) FragColor = vec4(lineColor, 1.0);
    else FragColor = vec4(1.0);
}