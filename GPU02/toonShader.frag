#version 150 core

uniform sampler2D renderTex;
uniform int useSobel;
uniform float edgeThresh;

const vec3 lum = vec3(0.2126, 0.7152, 0.0722);

in vec2	uv;
out vec4 out_Color;

float luminance(vec3 color){
	return dot(lum, color);
}

vec3 RGB2XYZ(vec3 color) {

	mat3 RGB_to_XYZ = mat3(
		0.49000, 0.31000, 0.20000,
		0.17697, 0.81240, 0.01063,
		0.00000, 0.01000, 0.99000);

    return RGB_to_XYZ * color;
}

vec3 XYZ2xyY(vec3 color) {
    float x = color.x / (color.x + color.y + color.z);
    float y = color.y / (color.x + color.y + color.z);
    float Y = color.y;
    return vec3(x, y, Y);
}

vec3 xyY2XYZ(vec3 xyYColor) {
    float X = xyYColor.x * (xyYColor.z / xyYColor.y);
    float Y = xyYColor.z;
    float Z = (1.0 - xyYColor.x - xyYColor.y) * (xyYColor.z / xyYColor.y);
    return vec3(X, Y, Z);
}

vec3 XYZ2RGB(vec3 color) {

  mat3 XYZ_to_RGB = mat3(
	2.36461385, -0.89654057, -0.46807328,
	-0.51516621, 1.4264081, 0.0887581,
	0.0052037, -0.01440816, 1.00920446);
  return XYZ_to_RGB * color;
}

vec3 quantizeColor(vec3 xyYColor) {
    float Yval = xyYColor.z;

	if(Yval<0.25) xyYColor.z = 0.125;
	else if (Yval>=0.25 && Yval<0.5) xyYColor.z = 0.375;
	else if (Yval>=0.5  && Yval<0.75) xyYColor.z = 0.675;
	else xyYColor.z = 0.875;

    return xyYColor;
}

vec4 sobel() {
	ivec2 pix = ivec2(gl_FragCoord.xy);

	float s00 = luminance( texelFetchOffset(renderTex, pix, 0, ivec2(-1,1)).rgb);
	float s10 = luminance( texelFetchOffset(renderTex, pix, 0, ivec2(-1,0)).rgb);
	float s20 = luminance( texelFetchOffset(renderTex, pix, 0, ivec2(-1,-1)).rgb);
	float s01 = luminance( texelFetchOffset(renderTex, pix, 0, ivec2(0,1)).rgb);
	float s21 = luminance( texelFetchOffset(renderTex, pix, 0, ivec2(0,-1)).rgb);
	float s02 = luminance( texelFetchOffset(renderTex, pix, 0, ivec2(1,1)).rgb);
	float s12 = luminance( texelFetchOffset(renderTex, pix, 0, ivec2(1,0)).rgb);
	float s22 = luminance( texelFetchOffset(renderTex, pix, 0, ivec2(1,-1)).rgb);

	float sx = s00 + 2 * s10 + s20 - (s02 + 2 * s12 + s22);
	float sy = s00 + 2 * s01 + s02 - (s20 + 2 * s21 + s22);
	float g = sx * sx + sy * sy;

	if (g >= edgeThresh) return vec4(0.0);
	else return vec4(-1.0); 
}

vec4 toonShade(){
	vec3 texColor = texture(renderTex, uv).rgb;
	vec3 XYZColor = RGB2XYZ(texColor);
	vec3 xyYColor = XYZ2xyY(XYZColor);
	xyYColor = quantizeColor(xyYColor);
	vec3 quantizedXYZColor = xyY2XYZ(xyYColor);
	vec3 finalColor = XYZ2RGB(quantizedXYZColor);		
	return vec4(finalColor, 1.0);
}

void main(void) {

	if(useSobel==1){ // use sobel filter
		vec4 edgeColor = sobel();
		if(edgeColor.r != -1.0){ // edge
			out_Color = edgeColor; 
		} else { // No edge
			out_Color = vec4(toonShade().rgb, 1.0); 
		}
	} else if (useSobel == 0) { // no sobel filter
		out_Color = toonShade();
	}

}
