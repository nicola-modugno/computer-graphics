#pragma once
#include "lib.h"
#include "gestione_callback.h"

extern GLFWwindow* window;
extern float    r, g, b;
extern double   xpos, ypos;
extern int      width, height;
extern int      mouseOverIndex;
extern bool     isMovingPoint;

// process keyboard input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

    // Se il tasto ESCAPE � premuto, chiude la finestra
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    switch (key) {
    case GLFW_KEY_F:
        if (action == GLFW_PRESS) {
            removeFirstPoint();
        }    break;
    case GLFW_KEY_L:
        if (action == GLFW_PRESS) {
            removeLastPoint();
        }   break;
    default:
        break;
    }
}

// glfw: whenever the mouse moves, this callback is called
void cursor_position_callback(GLFWwindow* window, double x, double y) {
    // (x,y) viewport(0,width)x(0,height)   -->   (xPos,yPos) window(-1,1)x(-1,1)
    float xPos = -1.0f + ((float)x) * 2 / ((float)(width));
    float yPos = -1.0f + ((float)(height - y)) * 2 / ((float)(height));

    findPoint(xPos, yPos);
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (isMovingPoint)
            modifyPoint(xPos, yPos);
        return;
    }
    // Visualizza le coordinate del mouse che si muove sulla finestra grafica    
    //   std::cout << "Mouse Coordinates : x = " << xpos << ", y = " << ypos << std::endl;
}

// glfw: whenever the mouse button is pressed or released
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        // (x,y) viewport(0,width)x(0,height)   -->   (xPos,yPos) window(-1,1)x(-1,1)
        double x, y;
        //getting cursor position
        glfwGetCursorPos(window, &x, &y);
        float xPos = -1.0f + ((float)x) * 2 / ((float)(width));
        float yPos = -1.0f + ((float)(height - y)) * 2 / ((float)(height));

        if (mouseOverIndex != -1) {
            isMovingPoint = true;
            return;
        }
        else {
            addNewPoint(xPos, yPos);
        }
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    //  Print new dimensions of the resized window
    std::cout << "Window size width: " << width << " height " << height << std::endl;
    glViewport(0, 0, width, height);
}