 #version 400

uniform int	pass;

in float	ndotv;
in vec3 GPosition;
in vec3 GNormal;
flat in int GIsEdge;

out vec4 FragColor;

const int levels = 3;
const float scaleFactor = 1.0 / levels;

vec3 toonShade( )
{
    return vec3(1.0) * ndotv;
}

void main() {
    if( GIsEdge == 1 ) {
        FragColor = vec4(0.0);
    } else {
        FragColor =  vec4( toonShade(), 1.0 );
    }

}