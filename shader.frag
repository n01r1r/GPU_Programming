 #version 400

uniform int	pass;

in float	ndotv;
out vec4	out_Color;

in vec3 Gpos;
in vec3 Gnorm;
flat in int GIsEdge;

vec4 gouraudShading(){
	return vec4(1.0) * ndotv;
}

vec4 pass2(){
	if(ndotv <= 0.25)	return vec4(0.0);
	else				return vec4(1.0);
}

void sillhouette(){
	
	if(GIsEdge == 1) out_Color = vec4(0.0);
	else			out_Color = gouraudShading();

}

void main(){
	if(pass==1) out_Color = gouraudShading();
	if(pass==2) out_Color = pass2();
	if(pass==3) sillhouette();
}