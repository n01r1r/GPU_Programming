 #version 150 core

uniform vec3		lightPosition;
uniform vec3		cameraPosition;
uniform vec3		diffColor;
uniform vec3		specColor;
uniform float		shininess;

uniform sampler2D	diffTex;
uniform sampler2D	bumpTex;

in vec3		pass_Normal;
in vec3		pass_WorldPos;
in vec2		pass_TexCoord;
out vec4	out_Color;

mat3 TBN(vec3 N){
	vec3 Q1		= dFdx(pass_WorldPos);
	vec3 Q2		= dFdy(pass_WorldPos);
	vec2 st1	= dFdx(pass_TexCoord);
	vec2 st2	= dFdy(pass_TexCoord);
	
	float D = st1.s * st2.t - st2.s * st1.t;
	return mat3( normalize(( Q1 * st2.t - Q2 * st1.t) * D),
				 normalize((-Q1 * st2.s + Q2 * st1.s) * D), 
				 N );
}

void main(void)
{	
	vec3 toLight =  lightPosition - pass_WorldPos;

	vec3 L = normalize(toLight);
	vec3 N = normalize(pass_Normal);
	float Bu = texture(bumpTex, pass_TexCoord + vec2(0.0001, 0)).r - texture(bumpTex, pass_TexCoord - vec2(0.0001, 0)).r;
	float Bv = texture(bumpTex, pass_TexCoord + vec2(0, 0.0001)).r - texture(bumpTex, pass_TexCoord - vec2(0, 0.0001)).r;
	vec3 bumpVec = vec3(-Bu*50, Bv*50, 1);
	mat3 tbn = TBN(N);
	N = normalize(tbn*bumpVec);
	vec3 V = normalize(cameraPosition - pass_WorldPos);
	vec3 R = reflect(-L, N);
	vec3 I_i = vec3(200, 200, 200) / dot(toLight, toLight);
	vec3 K_d = texture(diffTex, pass_TexCoord).rgb; 
	vec3 color = I_i * (max(0, dot(N, L)) * (K_d + specColor * pow(max(0, dot(R, V)), shininess)));
	out_Color = vec4(pow(color, vec3(1/2.2)), 1.0);

} 
