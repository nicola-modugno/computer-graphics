#pragma once
#include "lib.h"
 
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xpos, double ypos);
void removeLastPoint();
void removeFirstPoint();
void addNewPoint(float x, float y);
void findPoint(float xPos, float yPos);