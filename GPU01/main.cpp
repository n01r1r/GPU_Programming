/*
* GPU Programming Assignment #01
* Morph 3D models to spheres
* 2024/03/21, Dongyeob Han
*/

#include <iostream>
#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_transform.hpp> //from GLM 1.0~

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "toys.h"
#include "j3a.hpp"

using namespace glm;

void render(GLFWwindow* window, float time);
void init();

Program program;

GLuint vBuffer = 0;
GLuint idxBuffer = 0;
GLuint normBuffer = 0;
GLuint texBuffer = 0;
GLuint vArray = 0;
GLuint diffTex = 0;
GLuint bumpTex = 0;

const float PI = 3.14159265358979;

float theta = 0.f;
float phi	= 0.f;
float fov	= 45.f;

int mode = 0;
float uTime = 0.f;
bool isIncreasing = true;

double lastX	= 0;
double lastY	= 0;
double t_x		= 0;
double t_y		= 0;

vec3 lightPosition = vec3(3, 10, 10);

void cursorCB(GLFWwindow* w, double xpos, double ypos) {

	if (glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_1)) {

		// camera rotation for left mouse click
		theta -= (xpos - lastX) / 150;
		phi -= (ypos - lastY) / 150;

	}

	else if (glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_2)) {

		// camera translation for right mouse click
		t_x += (xpos - lastX) / 15;
		t_y += (ypos - lastY) / 15;

	}

	lastX = xpos;
	lastY = ypos;

}

void mouseBtn1CB(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
		glfwGetCursorPos(window, &lastX, &lastY);

	}
}

void mouseBtn2CB(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_PRESS) {
		glfwGetCursorPos(window, &lastX, &lastY);

	}
}

void scrollCB(GLFWwindow* window, double xoffset, double yoffset) {
	// zoom
	fov -= yoffset;
}

int main(void) {
	if (!glfwInit()) {
		fprintf(stderr, "glfwInit() failed");
		return -1;
	}

	GLFWwindow* window = glfwCreateWindow(640, 480, "test", NULL, NULL);

	if (!window) {
		fprintf(stderr, "GLFW Window Failed");
		glfwTerminate();
		return -1;
	}

	glfwSetCursorPosCallback(window, cursorCB);
	glfwSetMouseButtonCallback(window, mouseBtn1CB);
	glfwSetMouseButtonCallback(window, mouseBtn2CB);
	glfwSetScrollCallback(window, scrollCB);
	glfwMakeContextCurrent(window);

	glewInit();
	init();

	std::cout << "\n\nEnter what you want to view: \n";
	std::cout << "Press 1 to view animation, Press 2 to enter specific offset\n";
	std::cout << "Select Mode:   ";
	std::cin >> mode;

	if (mode == 1) {
		while (!glfwWindowShouldClose(window)) {
			// 1. Animation
			if (!isIncreasing) {
				if (uTime <= 0) {
					isIncreasing = true;
				}
				else uTime -= 0.0005;
			}
			else {
				if (uTime >= 1) {
					isIncreasing = false;
				}
				else uTime += 0.0005;
			}
			render(window, uTime);
			glfwPollEvents();
		}
	}

	else if (mode == 2) {
		// 2. Offset
		float offset = 0;
		std::cout << "\n Enter offset value: ";			
		std::cin >> offset;
		while (!glfwWindowShouldClose(window)) {
			render(window, offset);
			glfwPollEvents();
		}
	}
	return 0;
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

void init() {

	loadJ3A("./dwarf.j3a");
	program.loadShaders("shader.vert", "shader.frag");
	diffTex = loadTextures(diffuseMap[0]);
	bumpTex = loadTextures(bumpMap[0]);

	glGenBuffers(1, &vBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
	glBufferData(GL_ARRAY_BUFFER, nVertices[0] * sizeof(vec3), vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &idxBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, nTriangles[0] * sizeof(u32vec3), triangles[0], GL_STATIC_DRAW);

	glGenBuffers(1, &normBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normBuffer);
	glBufferData(GL_ARRAY_BUFFER, nVertices[0] * sizeof(vec3), normals[0], GL_STATIC_DRAW);

	glGenBuffers(1, &vArray);
	glBindVertexArray(vArray);

	glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, 0, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, normBuffer);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, 0, 0, 0);

}


void render(GLFWwindow* window, float time) {

	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	glViewport(0, 0, w, h);

	vec3 cameraPosVec = vec3(0, 0, 8);
	mat4 cameraPosMat = rotate(mat4(1), phi, vec3(1, 0, 0)) * rotate(mat4(1), theta, vec3(0, 1, 0));
	cameraPosVec = vec3(cameraPosMat * vec4(cameraPosVec, 1));
	mat4 projMat = perspective(radians(fov), w / float(h), 0.1f, 100.f);
	mat4 viewMat = lookAt(cameraPosVec, vec3(0), vec3(0, 1, 0));
	mat4 modelMat = mat4(1);
	projMat += mat4(vec4(0), vec4(0), vec4(0), vec4(t_x, -t_y, 0, 0));
	
	//std::cout << "\n" << uTime;

	glEnable(GL_DEPTH_TEST); // Depth Test Enable하면
	glClearColor(.7, .7, .7, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Depth Buffer도 Clear해줘야 한다

	glUseProgram(program.programID);

	GLuint loc = glGetUniformLocation(program.programID, "projMat");
	glUniformMatrix4fv(loc, 1, false, glm::value_ptr(projMat));
	loc = glGetUniformLocation(program.programID, "viewMat");
	glUniformMatrix4fv(loc, 1, false, glm::value_ptr(viewMat));
	loc = glGetUniformLocation(program.programID, "modelMat");
	glUniformMatrix4fv(loc, 1, false, glm::value_ptr(modelMat));
	loc = glGetUniformLocation(program.programID, "lightPosition");
	glUniform3fv(loc, 1, glm::value_ptr(lightPosition));
	loc = glGetUniformLocation(program.programID, "cameraPosition");
	glUniform3fv(loc, 1, glm::value_ptr(cameraPosVec));
	loc = glGetUniformLocation(program.programID, "diffColor");
	glUniform3fv(loc, 1, glm::value_ptr(diffuseColor[0]));
	loc = glGetUniformLocation(program.programID, "specColor");
	glUniform3fv(loc, 1, glm::value_ptr(specularColor[0]));
	loc = glGetUniformLocation(program.programID, "shininess");
	glUniform1f(loc, shininess[0]);
	loc = glGetUniformLocation(program.programID, "time");
	glUniform1f(loc, time);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffTex);
	loc = glGetUniformLocation(program.programID, "diffTex");
	glUniform1i(loc, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bumpTex);
	loc = glGetUniformLocation(program.programID, "bumpTex");
	glUniform1i(loc, 1);

	glBindVertexArray(vArray);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxBuffer);
	glDrawElements(GL_TRIANGLES, nTriangles[0] * 3, GL_UNSIGNED_INT, 0);
	glfwSwapBuffers(window);
}
