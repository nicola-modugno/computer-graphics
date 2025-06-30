/* LAB_2_2D_JUMP_BALL.cpp :
 *
 *     This program draws simple animation of a jumping ball
 * Usage:
 *	 Press "a" to move left
 *	 Press "d" to move right
 *	 Press escape to exit.
 */

#include "lib.h"
#include "ShaderMaker.h"
#include "gestione_callback.h"

static double limitFPS = 1.0 / 60.0;

typedef struct { float x, y, r, g, b, a; } Pointxy;

void disegna_pianoxy(float x, float y, float width, float height, vec4 color_top, vec4 color_bot, Pointxy* piano);
void disegna_PalaEolica(int nTriangles, Pointxy* PalaEolica);
void disegna_palla(int nTriangles, Pointxy* Palla);
void disegna_sole(int nTriangles, Pointxy* Sole);
void disegna_montagne(float x0, float y0, int altezza_montagna, int larghezza_montagne, int numero_di_montagne, Pointxy * v_montagna);

// viewport size
int     width = 640;
int     height = 480; 
mat4    Projection;  //Matrice di proiezione
mat4    Model;       //Matrice di modellazione

// Variabili globali -------------------------------------------------------------------
static unsigned int programId;

// Oggetti - Geometria
unsigned int VAO_MONTAGNE, VAO_PRATO, VAO_SOLE, VAO_CIELO, VAO_PALLA, VAO_PalaEolica;
unsigned int VBO_M, VBO_P, VBO_S, VBO_C, VBO_Pa, VBO_PE, MatProj, MatModel;
extern float angolo;
extern int Num_strip_instances;
extern int nVertices_montagna;
extern Pointxy* Punti;
extern int vertices_Prato;
extern Pointxy* Prato;
extern int vertices_Cielo;
extern Pointxy* Cielo;
extern int nTriangles_sole;
extern int vertices_sole;
extern Pointxy* Sole;
extern int nTriangles_palla;
extern int vertices_palla;
extern Pointxy* Palla;
extern int nTriangles_PalaEolica;
extern int vertices_PalaEolica;
extern Pointxy* PalaEolica;


// parametri della palla
float	distacco_da_terra = 0;	    // distacco da terra 
float	delta = 15.0;
float	Muovi_Orizzontale = 0.0;    // velocita orizzontale (pixel per frame)
float	posx = float(width) / 2.0f; // posizione iniziale della palla
float	posy = float(height) * 0.2f;

bool    pressing_left = false;
bool    pressing_right = false;

//Interpolazione lineare tra a e b con parametro amount
float lerp(float a, float b, float amount) {    
    return (1 - amount) * a + amount * b;
}
/************************************************************/
void init(void)
{
    Projection = ortho(0.0f, float(width), 0.0f, float(height));
    MatProj = glGetUniformLocation(programId, "Projection");
    MatModel = glGetUniformLocation(programId, "Model");
      
    //Gestione trasparenza : canale colore alpha 
    glEnable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    //Costruzione geometria e colori del CIELO
    vec4 col_top = { 0.3,0.6,1.0,1.0 };
    vec4 col_bottom = { 0.0 , 0.1, 1.0, 1.0 };
    disegna_pianoxy(0, height * 0.5f, (GLfloat) width, height * 0.5f, col_bottom, col_top, Cielo);
    //Generazione del VAO del Cielo
    glGenVertexArrays(1, &VAO_CIELO);
    glBindVertexArray(VAO_CIELO);
    glGenBuffers(1, &VBO_C);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_C);
    
    glBufferData(GL_ARRAY_BUFFER, vertices_Cielo * sizeof(Pointxy), &Cielo[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    
    //Costruzione geometria e colori del PRATO
    col_bottom = { 0.1, 0.5, 0.1, 1.0 };
    col_top = { 0.8, 1.0, 0.2, 1.0 };
    disegna_pianoxy(0.0f, 0.0f, (GLfloat) width, (GLfloat)height * 0.5f, col_bottom, col_top, Prato);
    //Genero un VAO Prato
    glGenVertexArrays(1, &VAO_PRATO);
    glBindVertexArray(VAO_PRATO);
    glGenBuffers(1, &VBO_P);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_P);
    
    glBufferData(GL_ARRAY_BUFFER, vertices_Prato * sizeof(Pointxy), &Prato[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    //Costruzione geometria e colori del SOLE
    //Genero il VAO del SOLE
    disegna_sole(nTriangles_sole, Sole);
    glGenVertexArrays(1, &VAO_SOLE);
    glBindVertexArray(VAO_SOLE);
    glGenBuffers(1, &VBO_S);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_S);
    
    glBufferData(GL_ARRAY_BUFFER, vertices_sole * sizeof(Pointxy), &Sole[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    //Costruzione geometria e colori delle MONTAGNE
    disegna_montagne(0, 0, 100, width, 3, Punti);
    //Genero un VAO Montagne
    glGenVertexArrays(1, &VAO_MONTAGNE);
    glBindVertexArray(VAO_MONTAGNE);
    glGenBuffers(1, &VBO_M);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_M);
    
    glBufferData(GL_ARRAY_BUFFER, nVertices_montagna * sizeof(Pointxy), &Punti[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    //Costruzione geometria e colori della PALLA
    disegna_palla(nTriangles_palla, Palla);
    glGenVertexArrays(1, &VAO_PALLA);
    glBindVertexArray(VAO_PALLA);
    glGenBuffers(1, &VBO_Pa);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Pa);
    
    glBufferData(GL_ARRAY_BUFFER, vertices_palla * sizeof(Pointxy), &Palla[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    //Costruzione geometria e colori delle PALE EOLICHE
    disegna_PalaEolica(nTriangles_PalaEolica, PalaEolica);
    glGenVertexArrays(1, &VAO_PalaEolica);
    glBindVertexArray(VAO_PalaEolica);
    glGenBuffers(1, &VBO_PE);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_PE);
    
    glBufferData(GL_ARRAY_BUFFER, vertices_PalaEolica * sizeof(Pointxy), &PalaEolica[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // Background color
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glViewport(0, 0, width, height);
}

//Movimento della palla in orizzontale
void update()
{
    bool moving = false;
    
    if (pressing_left) 
    {
        Muovi_Orizzontale -= 1;
        moving = true;
    }
    if (pressing_right)
    {
        Muovi_Orizzontale += 1;
        moving = true;
    }
    if (!moving) {// Se non sono premuti i tasti a e d,
                  // la palla continua a rimbalzare sul posto 
                  // decremento o incremento della velocita' iniziale fino a portarla a zero.
                  // NON IMPLEMENTATA
    }

    //Aggioramento della posizione in x della pallina
    posx += Muovi_Orizzontale;

    //Se la palla urta i bordi dello schermo 
    // ovvero assume una posizione x<0 o x> width (bordi viewport)
    // la pallina rimbalza ai bordi dello schermo attenuando la velocità
    if (posx < 0.0) {
        posx = 0.0f;
        Muovi_Orizzontale = -Muovi_Orizzontale * 0.8f;
    }
    if (posx > width) {
        posx = (float)width;
        Muovi_Orizzontale = -Muovi_Orizzontale * 0.8f;
    }

    // Gestione del rimbalzo in relazione all'altezza da terra
    // Rimbalzo -20 < distacco_da_terra < 100
    // lo spostamento delta è diminuito proporzionalmente al distacco da terra
    distacco_da_terra += (delta * (1.0f - (distacco_da_terra / 120.0f)));
    // invertire direzione del jump
    if ((distacco_da_terra > 100) && (delta > 0))
        delta = -delta;
    if ((distacco_da_terra < -20) && (delta < 0))
        delta = -delta;
}

void initShader(void)
{
    GLenum ErrorCheckValue = glGetError();

    char* vertexShader = (char*)"vertexshaderC.glsl";
    char* fragmentShader = (char*)"fragmentshaderC.glsl";
    // La funzione successiva crea un programma shader completo a partire da due shader individuali: 
    // uno per la gestione dei vertici e uno per la gestione dei pixel. 
    // Il programma shader risultante viene identificato da un identificativo univoco (il programId) 
    programId = ShaderMaker::createProgram(vertexShader, fragmentShader);
    //La funzione successiva "attiva" il programma shader associato all'identificatore programId.
    glUseProgram(programId);
}
void drawScene(void)
{
    // Background color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUniformMatrix4fv(MatProj, 1, GL_FALSE, value_ptr(Projection));
    
    //Disegna cielo
    Model = mat4(1.0);
    glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
    glBindVertexArray(VAO_CIELO);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawArrays(GL_TRIANGLES, 0, vertices_Cielo);
    glBindVertexArray(0);

    //Disegna prato
    Model = mat4(1.0);
    glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
    glBindVertexArray(VAO_PRATO);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawArrays(GL_TRIANGLES, 0, vertices_Prato);
    glBindVertexArray(0);

    // Disegna sole
    Model = mat4(1.0);
    Model = translate(Model, vec3(float(width) * 0.5, float(height) * 0.8, 0.0));
    Model = scale(Model, vec3(30.0, 30.0, 1.0));
    glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
    glBindVertexArray(VAO_SOLE);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawArrays(GL_TRIANGLES, 0, vertices_sole / 2);
    //Disegna Alone del sole
    Model = mat4(1.0);
    Model = translate(Model, vec3(float(width) * 0.5, float(height) * 0.8, 0.0));
    Model = scale(Model, vec3(80.0, 80.0, 1.0));
    glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
    glDrawArrays(GL_TRIANGLES, vertices_sole / 2, vertices_sole / 2);
    glBindVertexArray(0);

    //Disegna montagne
    Model = mat4(1.0);
    Model = translate(Model, vec3(0.0, float(height) / 2, 0.0));
    glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
    glBindVertexArray(VAO_MONTAGNE);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, nVertices_montagna);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBindVertexArray(0);

    // Disegna palla (ombra+palla)
    Model = mat4(1.0);
    // larghezza effettiva in pixel della palla (80 diametro palla normale -- 100 palla dilatata)
    double bwidth = distacco_da_terra < 0 ? lerp(80.0f, 100.0f, (float)abs(distacco_da_terra) / 20) : 80;
    // altezza effettiva in pixel della palla
    double bheight = distacco_da_terra < 0 ? 80 + distacco_da_terra : 80;
    float shadow_scale = lerp(1.0f, 0.0f, (float)distacco_da_terra / 255); //distacco grande -> fattore scala piccolo
    //Matrice per il cambiamento di posizione dell'OMBRA della palla
    Model = translate(Model, vec3(posx - bwidth / 2 * shadow_scale, posy + 10 + 10 * (1 - shadow_scale), 0.0f));
    Model = scale(Model, vec3(float(bwidth) * shadow_scale, (50.0 * shadow_scale), 1.0));
    glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
    glBindVertexArray(VAO_PALLA);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawArrays(GL_TRIANGLES, vertices_palla / 2, vertices_palla / 2);
    //Matrice per il cambiamento di posizione della  PALLA
    Model = mat4(1.0);
    Model = translate(Model, vec3(posx - bwidth / 2, posy + bheight + distacco_da_terra, 0.0f));
    Model = scale(Model, vec3(float(bwidth) / 2, float(bheight) / 2, 1.0));
    glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawArrays(GL_TRIANGLES, 0, vertices_palla / 2);
    glBindVertexArray(0);

    //Disegna Pala Eolica
    glBindVertexArray(VAO_PalaEolica);
    for (int i = 1; i < 7; i++) /* Disegna 6 istanze di Pala Eolica (sostegno+pale) */
    {
        Model = mat4(1.0);
        Model = translate(Model, vec3(float(width) * 0.15 * i, float(height) * 0.62, 0.0));
        Model = scale(Model, vec3(3.0, 100.0, 1.0));
        glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
        glDrawArrays(GL_TRIANGLES, vertices_PalaEolica - 6, 6); //Supporto
        Model = mat4(1.0);
        Model = translate(Model, vec3(float(width) * 0.15 * i, float(height) * 0.62, 0.0));
        Model = scale(Model, vec3(40.0, 40.0, 1.0));
        Model = rotate(Model, radians(angolo), vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawArrays(GL_TRIANGLES, 0, vertices_PalaEolica - 6); // Pala (Starting from vertex 0)
    }
    glBindVertexArray(0);
}


// Function to initialize OpenGL and create a window.
GLFWwindow* initOpenGL() {

    // Initialize GLFW.
    bool glfw_state = glfwInit();
    /******* Inizializzazioni GLFW library *************/
    if (!glfw_state) {
        std::cout << "Failed to initialize GLFW." << std::endl;
        glfwTerminate();
    }

    //Imposta le proprietà del contesto e del profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //Abilita il double buffering
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

    /******* Create a window and its OpenGL context *****/
    GLFWwindow* window = glfwCreateWindow( width, height, "LAB_2_JUMP_BALL", NULL, NULL);
    if (!window)
    {
        std::cout << "Failed to create GLFW window !" << std::endl;
        glfwTerminate();
    }
    /* Make the window's context current */
    glfwMakeContextCurrent(window); //crea il context corrente e lo associa a window. In opengl un rendering context è una macchina astati che memorizza tutte le informazioni necessarie e le risorse per il rendering grafico

    // verifica se la libreria GLAD è riuscita a caricare correttamente tutti i puntatori 
    // alle funzioni OpenGL necessarie.
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to load opengl function pointers !" << std::endl;
        glfwTerminate();
    }
    return window;
}


int main(void)
{    
    // Initialize OpenGL and create a window.
    GLFWwindow* window = initOpenGL();
    initShader();
    init();
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    /******* Function callbacks : chiamate quando si verificano determinati eventi ******/
    // process keyboard input
    glfwSetKeyCallback(window, key_callback);
    // process mouse motion
    glfwSetCursorPosCallback(window, cursor_position_callback);
    // process mouse button pressed or released
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    // process window resize
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Gestione time
    int frames = 0, updates = 0;
    double lastTime = glfwGetTime(), timer = lastTime;
    double deltaTime = 0, nowTime = 0;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        // Measure time
        nowTime = glfwGetTime();
        deltaTime += (nowTime - lastTime) / limitFPS;
        lastTime = nowTime;

        // Only update at 60 frames / s
        while (deltaTime >= 1.0) {
            update();   // - Update function
            updates++;
            deltaTime--;
        }
        // Render 
        drawScene();
        frames++;

        // Reset after one second
        if (glfwGetTime() - timer > 1.0) {
            timer++;
            std::cout << "FPS: " << frames << " Number of Updates:" << updates << std::endl;
            updates = 0, frames = 0;
        }
        
        /* Swap front and back buffers */
        glfwSwapBuffers(window);
        /* Poll for and process events */
        glfwPollEvents();
    }

    glDeleteProgram(programId);
    glDeleteVertexArrays(1, &VAO_MONTAGNE);
    glDeleteVertexArrays(1, &VAO_PRATO);
    glDeleteVertexArrays(1, &VAO_CIELO);
    glDeleteVertexArrays(1, &VAO_PALLA);
    glDeleteVertexArrays(1, &VAO_PalaEolica);

    glfwTerminate(); //Libera le risorse allocate da glfwInit
    return 0;
}