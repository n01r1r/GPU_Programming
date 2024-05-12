#pragma once
#ifndef CALLBACK_H
#define CALLBACK_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

	float theta = 0.f;
	float phi = 0.f;
	float fov = 25.f;

	double lastX = 0;
	double lastY = 0;

	glm::vec3 cameraPosition = glm::vec3(0, 0, 8);

	void cursorCB(GLFWwindow* w, double xpos, double ypos) {
		if (glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_1)) {
			theta	-= (xpos - lastX) / 150;
			phi		-= (ypos - lastY) / 150;
		}
		lastX = xpos;
		lastY = ypos;
	}

	void mouseCB(GLFWwindow* w, int button, int action, int mods) {
		if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
			glfwGetCursorPos(w, &lastX, &lastY);
		}
	}

	void scrollCB(GLFWwindow* w, double xoffset, double yoffset) {
		fov -= yoffset;
	}


#endif