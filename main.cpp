/*
* GPU Programming Assignment #03
* Silhouette generation with geometry shader
* 2024/05/12, Dongyeob Han
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
#include "callback.h"
#include "toys.h"
#include "j3a.hpp"
#include <iostream>
#include <vector>

using namespace glm;


void initScene();
void renderScene(GLFWwindow* window);

const int screenWidth = 1200;
const int screenHeight = 800;

bool isBlur = false;

Program prog;

GLuint vArray = 0;
GLuint vBuffer = 0;
GLuint idxBuffer = 0;
GLuint normBuffer = 0;
GLuint texBuffer = 0;


vec3 lightPosition = vec3(3, 10, 5);


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
	glfwSetCursorPosCallback(window, cursorCB);
	glfwSetMouseButtonCallback(window, mouseCB);
	glfwSetScrollCallback(window, scrollCB);
	glfwMakeContextCurrent(window);
	glewInit();

	initScene();

	while (!glfwWindowShouldClose(window)) {
		renderScene(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

void initScene() {

	// load models and shader programs
	loadJ3A("./dwarf.j3a");
	prog.loadShaders("./shader.vert", "./shader.frag", "./shader.geom");

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

	glGenBuffers(1, &vArray);
	glBindVertexArray(vArray);

	glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, 0, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, normBuffer);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, 0, 0, 0);

}

void renderScene(GLFWwindow* window) {

	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	glViewport(0, 0, w, h);
	vec3 cameraPosition = vec3(0, 0, 8);
	cameraPosition = vec3(
		rotate(mat4(1), theta, vec3(0, 1, 0))
		* rotate(mat4(1), phi, vec3(1, 0, 0))
		* vec4(cameraPosition, 1)
	);

	mat4 projMat = perspective(radians(fov), w / float(h), 0.1f, 100.f);
	mat4 viewMat = lookAt(cameraPosition, vec3(0), vec3(0, 1, 0));
	mat4 modelMat = mat4(1);
	mat4 mv = viewMat * modelMat;
	mat4 normalMat = mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2]));

	glEnable(GL_DEPTH_TEST);
	glClearColor(.7, .7, .7, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

	GLuint loc = glGetUniformLocation(prog.programID, "projMat");
	glUniformMatrix4fv(loc, 1, false, glm::value_ptr(projMat));

	loc = glGetUniformLocation(prog.programID, "viewMat");
	glUniformMatrix4fv(loc, 1, false, glm::value_ptr(viewMat));

	loc = glGetUniformLocation(prog.programID, "modelMat");
	glUniformMatrix4fv(loc, 1, false, glm::value_ptr(modelMat));

	loc = glGetUniformLocation(prog.programID, "normalMat");
	glUniformMatrix3fv(loc, 1, false, glm::value_ptr(normalMat));

	loc = glGetUniformLocation(prog.programID, "lightPosition");
	glUniform3fv(loc, 1, glm::value_ptr(lightPosition));

	loc = glGetUniformLocation(prog.programID, "cameraPosition");
	glUniform3fv(loc, 1, glm::value_ptr(cameraPosition));

	loc = glGetUniformLocation(prog.programID, "diffColor");
	glUniform3fv(loc, 1, glm::value_ptr(diffuseColor[0]));

	loc = glGetUniformLocation(prog.programID, "specColor");
	glUniform3fv(loc, 1, glm::value_ptr(specularColor[0]));

	loc = glGetUniformLocation(prog.programID, "shininess");
	glUniform1f(loc, shininess[0]);

	loc = glGetUniformLocation(prog.programID, "pass");
	glUniform1i(loc, input);

	glBindVertexArray(vArray);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxBuffer);
	glDrawElements(GL_TRIANGLES, nTriangles[0] * 3, GL_UNSIGNED_INT, 0);
	glfwSwapBuffers(window);
}
