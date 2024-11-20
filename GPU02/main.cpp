/*
* GPU Programming Assignment #02
* Gaussian Filter, Edge Detection, Toon Shading
* implemented with multi-pass
* 2024/05/03, Dongyeob Han
*/

#define GLEW_STATIC
#include <GL/glew.h>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_transform.hpp> // from GLM 1.0~

// Textures
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Misc
//#include "callback.h"
#include "toys.h"
#include "j3a.hpp"
#include <iostream>
#include <vector>

using namespace glm;

struct FBO {
	GLuint framebuffer = 0;
	GLuint colorbuffer = 0;
	GLuint depthbuffer = 0;
};

void initScene();
void setupFBO();
void renderScene();
GLuint loadTextures(const std::string& filename);
void applyGaussian(FBO* inputFBO, GLuint outputFBO);
void toonShadeAndSobel(FBO* inputFBO, bool useSobel);

const int screenWidth = 1200;
const int screenHeight = 800;

bool isBlur = false;

FBO diffFBO, gaussianFBO, tempFBO, toonFBO;

Program diffProg, gaussianProg, toonProg;
Camera camera;

GLuint vArray = 0;
GLuint vBuffer = 0;
GLuint idxBuffer = 0;
GLuint normBuffer = 0;
GLuint texBuffer = 0;
GLuint diffTex = 0;
GLuint bumpTex = 0;

GLuint quadVertexBuffer = 0;
GLuint quadArrrayBuffer = 0;
float quadVertices[] = { // 화면 전체에 렌더링하기 위한 사각형 정점 데이터
	-1.0f,  1.0f, 0.0f,// 왼쪽 상단
	-1.0f, -1.0f, 0.0f,// 왼쪽 하단
	 1.0f, -1.0f, 0.0f,// 오른쪽 하단

	-1.0f,  1.0f, 0.0f,// 왼쪽 상단
	 1.0f, -1.0f, 0.0f,// 오른쪽 하단
	 1.0f,  1.0f, 0.0f,// 오른쪽 상단
};



vec3 lightPosition = vec3(3, 10, 5);
vec3 cameraPosVec = vec3(-0.6, 0, 2);
float fov = 75.f;

int input = 1;

void keyCB(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_G && action == GLFW_PRESS) isBlur = !isBlur;

	if (key == GLFW_KEY_1 && action == GLFW_PRESS) input = 1;

	if (key == GLFW_KEY_2 && action == GLFW_PRESS) input = 2;

	if (key == GLFW_KEY_3 && action == GLFW_PRESS) input = 3;

}


int main(void) {

	if (!glfwInit()) {
		fprintf(stderr, "glfwInit() failed");
		return -1;
	}

	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "GPU Assignment 02", NULL, NULL);

	if (!window) {
		fprintf(stderr, "GLFW Window Failed");
		glfwTerminate();
		return -1;
	}

	glfwSetKeyCallback(window, keyCB);
	glfwMakeContextCurrent(window);
	glewInit();

	initScene();
	setupFBO();

	while (!glfwWindowShouldClose(window)) {
		glViewport(0, 0, screenWidth, screenHeight);
		glClearColor(.25, .25, .25, 0);

		

		switch (input) {
		case 1:
			if (isBlur) {
				glBindFramebuffer(GL_FRAMEBUFFER, diffFBO.framebuffer);
				renderScene();
				applyGaussian(&diffFBO, 0);
			} 
			else {
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				renderScene();
			}
			break;
		case 2:
			if (isBlur) {
				glBindFramebuffer(GL_FRAMEBUFFER, diffFBO.framebuffer);
				renderScene();
   				applyGaussian(&diffFBO, gaussianFBO.framebuffer);
				toonShadeAndSobel(&gaussianFBO, false);
			}
			else {
				glBindFramebuffer(GL_FRAMEBUFFER, diffFBO.framebuffer);
				renderScene();
				toonShadeAndSobel(&diffFBO, false);
			}
			break;
		case 3:
			if (isBlur) {
				glBindFramebuffer(GL_FRAMEBUFFER, diffFBO.framebuffer);
				renderScene();
				applyGaussian(&diffFBO, gaussianFBO.framebuffer);
				toonShadeAndSobel(&gaussianFBO, true);
			}
			else {
				glBindFramebuffer(GL_FRAMEBUFFER, diffFBO.framebuffer);
				renderScene();
				toonShadeAndSobel(&diffFBO, true);
			}
			break;
		}
	
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

void setupFBO() {

	// screen quad VAO
	glGenBuffers(1, &quadVertexBuffer);
	glGenVertexArrays(1, &quadArrrayBuffer);
	glBindVertexArray(quadArrrayBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quadVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, 0, 0, 0);

	// first fbo setup (render scene)
	glGenFramebuffers(1, &diffFBO.framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, diffFBO.framebuffer);
	glGenTextures(1, &diffFBO.colorbuffer);
	glBindTexture(GL_TEXTURE_2D, diffFBO.colorbuffer);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, screenWidth, screenHeight);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, diffFBO.colorbuffer, 0);

	glGenRenderbuffers(1, &diffFBO.depthbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, diffFBO.depthbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenWidth, screenHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, diffFBO.depthbuffer);

	glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind and revert to default

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Diffuse FBO is not complete!" << std::endl;


	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };

	// second fbo (blur scene)
	glGenFramebuffers(1, &gaussianFBO.framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gaussianFBO.framebuffer);
	glGenTextures(1, &gaussianFBO.colorbuffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gaussianFBO.colorbuffer);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, screenWidth, screenHeight);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gaussianFBO.colorbuffer, 0);

	glDrawBuffers(1, drawBuffers);
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind and revert to default

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Gaussian FBO is not complete!" << std::endl;

	// third fbo (toon shade + sobel)
	glGenFramebuffers(1, &toonFBO.framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, toonFBO.framebuffer);
	glGenTextures(1, &toonFBO.colorbuffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, toonFBO.colorbuffer);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, screenWidth, screenHeight);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, toonFBO.colorbuffer, 0);

	glDrawBuffers(1, drawBuffers);
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind and revert to default

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Toon FBO is not complete!" << std::endl;

	// intermediate fbo
	glGenFramebuffers(1, &tempFBO.framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, tempFBO.framebuffer);
	glGenTextures(1, &tempFBO.colorbuffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tempFBO.colorbuffer);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, screenWidth, screenHeight);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tempFBO.colorbuffer, 0);

	glDrawBuffers(1, drawBuffers);
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind and revert to default

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Temp FBO is not complete!" << std::endl;
}

void initScene() {

	// load models and shader programs
	loadJ3A("./dwarf.j3a");
	diffProg.loadShaders("./diffuse.vert", "./diffuse.frag");
	gaussianProg.loadShaders("./drawquad.vert", "./gaussian.frag");
	toonProg.loadShaders("./drawquad.vert", "./toonShader.frag");

	// Scene setup and render
	glGenBuffers(1, &vBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
	glBufferData(GL_ARRAY_BUFFER, nVertices[0] * sizeof(vec3), vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &idxBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, nTriangles[0] * sizeof(u32vec3), triangles[0], GL_STATIC_DRAW);

	glGenBuffers(1, &normBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normBuffer);
	glBufferData(GL_ARRAY_BUFFER, nVertices[0] * sizeof(vec3), normals[0], GL_STATIC_DRAW);

	glGenBuffers(1, &texBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, texBuffer);
	glBufferData(GL_ARRAY_BUFFER, nVertices[0] * sizeof(vec2), texCoords[0], GL_STATIC_DRAW);

	glGenBuffers(1, &vArray);
	glBindVertexArray(vArray);

	glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, 0, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, normBuffer);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, 0, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, texBuffer);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, 0, 0, 0);

	// texture load
	diffTex = loadTextures(diffuseMap[0]);
	bumpTex = loadTextures(bumpMap[0]);
	
	
}

GLuint loadTextures(const std::string& filename) {
	GLuint tex = 0;
	int w, h, n;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* buf = stbi_load(filename.c_str(), &w, &h, &n, 4);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(buf);
	return tex;
}


void renderScene() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glUseProgram(diffProg.programID);

	// render scene
	mat4 cameraPosMat = rotate(mat4(1), 0.f, vec3(1, 0, 0)) * rotate(mat4(1), 0.f, vec3(0, 1, 0));
	cameraPosVec = vec3(cameraPosMat * vec4(cameraPosVec, 1));
	mat4 projMat = perspective(radians(fov), screenWidth / float(screenHeight), 0.1f, 100.f);
	mat4 viewMat = lookAt(cameraPosVec, vec3(0), vec3(0, 1, 0));
	mat4 modelMat = mat4(1);

	GLuint loc = glGetUniformLocation(diffProg.programID, "projMat");
	glUniformMatrix4fv(loc, 1, false, glm::value_ptr(projMat));

	loc = glGetUniformLocation(diffProg.programID, "viewMat");
	glUniformMatrix4fv(loc, 1, false, glm::value_ptr(viewMat));

	loc = glGetUniformLocation(diffProg.programID, "modelMat");
	glUniformMatrix4fv(loc, 1, false, glm::value_ptr(modelMat));

	loc = glGetUniformLocation(diffProg.programID, "lightPosition");
	glUniform3fv(loc, 1, glm::value_ptr(lightPosition));

	loc = glGetUniformLocation(diffProg.programID, "cameraPosition");
	glUniform3fv(loc, 1, glm::value_ptr(cameraPosVec));

	loc = glGetUniformLocation(diffProg.programID, "diffColor");
	glUniform3fv(loc, 1, glm::value_ptr(diffuseColor[0]));

	loc = glGetUniformLocation(diffProg.programID, "specColor");
	glUniform3fv(loc, 1, glm::value_ptr(specularColor[0]));

	loc = glGetUniformLocation(diffProg.programID, "shininess");
	glUniform1f(loc, shininess[0]);

	loc = glGetUniformLocation(diffProg.programID, "pass");
	glUniform1i(loc, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffTex);
	loc = glGetUniformLocation(diffProg.programID, "diffTex");
	glUniform1i(loc, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bumpTex);
	loc = glGetUniformLocation(diffProg.programID, "bumpTex");
	glUniform1i(loc, 1);

	glBindVertexArray(vArray);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxBuffer);
	glDrawElements(GL_TRIANGLES, nTriangles[0] * 3, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void applyGaussian(FBO* inputFBO, GLuint outputFBO) {

	glUseProgram(gaussianProg.programID);

	// Gaussian 1st pass
	glBindFramebuffer(GL_FRAMEBUFFER, tempFBO.framebuffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, inputFBO->colorbuffer); // bind the rendered result from input FBO
	glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT);

	glUniform1i(glGetUniformLocation(gaussianProg.programID, "pass"), 1);

	//glBindVertexArray(0);
	//glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindVertexArray(quadArrrayBuffer);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);


	// Gaussian 2nd pass
	glBindFramebuffer(GL_FRAMEBUFFER, outputFBO); // revert to default framebuffer or render to other FBO
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tempFBO.colorbuffer);
	glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT);

	glUniform1i(glGetUniformLocation(gaussianProg.programID, "pass"), 2);

	glBindVertexArray(quadArrrayBuffer);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void toonShadeAndSobel(FBO* inputFBO, bool useSobel) {
	glUseProgram(toonProg.programID);

	glBindFramebuffer(GL_FRAMEBUFFER, 0); // revert to default framebuffer for rendering
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, inputFBO->colorbuffer); // bind the rendered result from input FBO
	glDisable(GL_DEPTH_TEST);
	//glClear(GL_COLOR_BUFFER_BIT);

	glUniform1f(glGetUniformLocation(toonProg.programID, "edgeThresh"), 0.5);
	glUniform1i(glGetUniformLocation(toonProg.programID, "renderTex"), 0);
	if (useSobel)	glUniform1i(glGetUniformLocation(toonProg.programID, "useSobel"), 1);
	else			glUniform1i(glGetUniformLocation(toonProg.programID, "useSobel"), 0);

	glBindVertexArray(quadArrrayBuffer);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}