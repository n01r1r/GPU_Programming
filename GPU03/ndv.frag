#version 410 core

in vec3 wnorm;
in vec3 wpos;
in float ndv;

uniform int	pass;

out vec4 FragColor;

vec4 toonShade(){
    return vec4(1.0) * ndv;
}

vec4 shadePerNdotV(){
    if(ndv<=0.25) return vec4(0.0);
    else return vec4(1.0);
}

void main(){
    if(pass==1) FragColor = toonShade();
    if(pass==2) FragColor = shadePerNdotV();
}