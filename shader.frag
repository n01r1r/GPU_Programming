#version 410 core

in GS_FS{
	in vec3 GNormal;
	in vec3 GPosition;
} gs_out;

uniform int	pass;
flat in int GIsEdge;
out vec4 FragColor;

void main() {
    if(GIsEdge==1) FragColor = vec4(0.5, 0.5, 0.0, 1.0);
    else FragColor = vec4(1.0);
}