#pragma once 
#include "lib.h"
#include "imgui\imgui.h"
#include "imgui\imgui_impl_glfw.h"
#include "imgui\imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <math.h>

//Variabili di tipo extern

//Variabili locali
bool isOverImGuiElement, value;

extern float game_timer;
extern bool game_over;
extern int punti;

void init_GUI(GLFWwindow* window) {

    IMGUI_CHECKVERSION(); // Verifica la compatibilità della versione di ImGui

    // Create ImGui context
    ImGui::CreateContext(); 

    // Get ImGui IO object to configure options
    ImGuiIO& io = ImGui::GetIO(); // Ottieni l'id IO per configurare opzioni come input, font e stili
    io.FontGlobalScale = 2.0f;    // Dimensione font

    // Set dark theme
    ImGui::StyleColorsDark(); 

    // Initialize ImGui for GLFW and OpenGL 
    ImGui_ImplGlfw_InitForOpenGL(window, true); 
    ImGui_ImplOpenGL3_Init("#version 450 core");
}

void my_interface(GLFWwindow* window)
{
    // Inizia il frame ImGui
    ImGui_ImplGlfw_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();   

    // Interfaccia timer
    ImGui::Begin("Timer", NULL,
        ImGuiWindowFlags_NoCollapse |      
        ImGuiWindowFlags_AlwaysAutoResize
    );
    ImGui::Text("Tempo rimasto: %.2f\nPunti: %d", game_timer, punti);
    ImGui::End();

    // Game over popup
    if (game_over) {
        ImGui::OpenPopup("Game Over");

        // Centro il popup
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("Game Over", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Punti: %d", punti);
            ImGui::EndPopup();
        }

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    // Renderizza ImGui (questo deve venire DOPO drawScene)
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void close_GUI() {
    ImGui_ImplOpenGL3_Shutdown(); // Chiude l'integrazione con OpenGL
    ImGui_ImplGlfw_Shutdown();    // Chiude l'integrazione con GLFW
    ImGui::DestroyContext();      // Distrugge il contesto ImGui
}