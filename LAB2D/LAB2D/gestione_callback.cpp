#pragma once
#include "lib.h"

extern GLFWwindow* window;
extern int      width, height;
extern bool     pressing_right, pressing_left, mouse_pressed;
float   mouse_pointer_x, mouse_pointer_y;
extern float posx, posy, palla_posx, palla_posy; // posizione iniziale della palla
extern bool game_over;


// process keyboard input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

    // Se il tasto ESCAPE è premuto, chiude la finestra
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    switch (key) {
    case GLFW_KEY_A:
        if (action == GLFW_PRESS) {
            pressing_left = true;
        };
        if (action == GLFW_RELEASE) {
            pressing_left = false;
        }    break; 
    case GLFW_KEY_D:
        if (action == GLFW_PRESS) {
            pressing_right = true;
        };
        if (action == GLFW_RELEASE) {
            pressing_right = false;
        }    break;
    default:
        break;
    }
}

// glfw: whenever the mouse moves, this callback is called
void cursor_position_callback(GLFWwindow* window, double x, double y) {
    // (x,y) viewport(0,width)x(0,height)   -->   (xPos,yPos) window(-1,1)x(-1,1)
    if (mouse_pressed) {
        posx = (float)x;
        posy = (float)(height - y);  // coordinate da viewport a finestra
        //distacco_da_terra = 0;       // evita effetto salto mentre trascini
        //Muovi_Orizzontale = 0;       // ferma il movimento orizzontale
    }
    // Visualizza le coordinate del mouse che si muove sulla finestra grafica    
    //   std::cout << "Mouse Coordinates : x = " << xpos << ", y = " << ypos << std::endl;
}

// glfw: whenever the mouse button is pressed or released
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !game_over) {
        double x, y;
        glfwGetCursorPos(window, &x, &y);

        float fx = (float)x;
        float fy = (float)(height - y);  // Coordinate world

        float dx = fx - posx;
        float dy = fy - (posy + 40); // Centro palla

        float distanza = sqrt(dx * dx + dy * dy);

        // Controlla se sei entro il raggio e sopra (ignora ombra)
        if (distanza <= 40.0f && dy >= -40.0f) {
            mouse_pressed = true;
        }
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE && !game_over) {
        mouse_pressed = false;
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
