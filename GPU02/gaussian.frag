#version 150 core

layout(binding=0) uniform sampler2D renderTex;
uniform int pass;
in vec2 uv;
out vec4 out_Color;
const float PI = 3.14159265358979;


void main(void) {

    float weight[3];
    float sigma = 1.5;
    float sum = 0.0;

    weight[0] = 1 / (sqrt(2.0*PI)*sigma);

    for(int i=1; i<= 2; i++) {
        weight[i] = exp(-float(i*i) / (2.0*sigma*sigma)) / (sqrt(2.0*PI)*sigma);
        sum += 2.0 * weight[i];
    }

    sum += weight[0]; 

    vec4 color = vec4(0.0);
    vec2 screenSize = textureSize(renderTex, 0);
    vec2 offset;

    if(pass == 1) { 
        for(int i = -2; i <= 2; i++) {
            offset = vec2(float(i) / screenSize.x, 0.0);
            color += texture(renderTex, uv + offset) * weight[abs(i)];
        }
    } 
    
    if(pass == 2) {
        for(int i = -2; i <= 2; i++) {
            offset = vec2(0.0, float(i) / screenSize.y);
            color += texture(renderTex, uv + offset) * weight[abs(i)];
        }
    }

    out_Color = color / sum;
}
