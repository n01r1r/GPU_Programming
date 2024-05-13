#version 150 core

uniform int	pass;

in float	ndotv;
in vec3 GPosition;
in vec3 GNormal;
flat in int GIsEdge;

out vec4 FragColor;

vec4 toonShade( )
{
    return vec4(1.0) * ndotv;
}

vec4 shadePerNdotV(){
    if(ndotv<=0.25) return vec4(0.0);
    else return vec4(1.0);
}

vec4 drawSilhouette(){
    
    if(GIsEdge==1) return vec4(0.0);
    else return vec4(1.0);


}

void main() {
    if(pass==1) FragColor = toonShade();
    if(pass==2) FragColor = shadePerNdotV();
    if(pass==3) FragColor = drawSilhouette();
}