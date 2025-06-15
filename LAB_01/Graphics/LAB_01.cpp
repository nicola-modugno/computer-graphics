/*
 * Lab-01.c
 *
 *     This program draws straight lines connecting dots placed with mouse clicks.
 *
 * Usage:
 *   Left click to place a control point.
 *		Maximum number of control points allowed is currently set at 300.
 *	 Press "f" to remove the first control point
 *	 Press "l" to remove the last control point.
 *	 Press escape to exit.
 */
#include "lib.h"
#include "ShaderMaker.h"
#include "gestione_callback.h"

 /*** Include GLM; libreria matematica per le opengl ***/
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

int		width = 800;
int		height = 800;

///////////////////////////////////////////////////////////
// Variabili globali -------------------------------------------------------------------
static unsigned int programId;

float r = 0.0, g = 0.5, b = 1.0;

unsigned int    vao, vposition_CP_ID;     // Control Polygon
unsigned int    vao_2, vposition_Curve_ID;  // Curve (TO BE computed)

//#define MaxNumPts 300
#define MaxNumPts 300

float   vPositions_CP[MaxNumPts][2];        // Control Polygon
float   vPositions_C[MaxNumPts][2];         // Curve (TO BE computed)

// Array per i punti di controllo Bézier generati
float vPositions_Bezier[MaxNumPts * 3][2];  // Punti di controllo per curve di Bézier a tratti
int NumBezierPts = 0;

int     NumPts = 0;
int     NumPoints = 0;
int     mouseOverIndex = -1;

bool     isMovingPoint = false;
//----------------------------------------------------------------------------------------
void removeFirstPoint() {
    int i;
    if (NumPts > 0) {
        // Remove the first point, slide the rest down
        NumPts--;
        for (i = 0; i < NumPts; i++) {
            vPositions_CP[i][0] = vPositions_CP[i + 1][0];
            vPositions_CP[i][1] = vPositions_CP[i + 1][1];
        }
    }
}
// Add a new point to the end of the list.  
// Remove the first point in the list if too many points.
void removeLastPoint() {
    if (NumPts > 0) {
        NumPts--;
    }
}
// Add a new point to the end of the list.  
// Remove the first point in the list if too many points.
void addNewPoint(float x, float y) {
    if (NumPts >= MaxNumPts) {
        removeFirstPoint();
    }
    vPositions_CP[NumPts][0] = x;
    vPositions_CP[NumPts][1] = y;
    printf("CP %i coords (%f, %f) \n", NumPts, vPositions_CP[NumPts][0], vPositions_CP[NumPts][1]);
    NumPts++;
    printf("Actual total of points: %i\n", NumPts+NumBezierPts);
}

float findDistance(float firstPoint_xPos, float firstPoint_yPos, float secondPoint_xPos, float secondPoint_yPos) {
    float dist;
    return sqrt(pow(firstPoint_xPos - secondPoint_xPos, 2) + pow(firstPoint_yPos - secondPoint_yPos, 2));
}

void findPoint(float xPos, float yPos) {
    float dist;
    for (int i = 0; i < NumPts; i++) {
        dist = findDistance(vPositions_CP[i][0], vPositions_CP[i][1], xPos, yPos);
        if (dist < 0.06) {
            mouseOverIndex = i;
            printf("Mouse over CP %i\n", i);
            return;
        }
        else
            mouseOverIndex = -1;
    }

}

void modifyPoint(float xPos, float yPos) {
    vPositions_CP[mouseOverIndex][0] = xPos;
    vPositions_CP[mouseOverIndex][1] = yPos;
}


void init(void)
{
    // Create a vertex array object for CONTROL POLYGON
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    // attribute VBO : positions 
    glGenBuffers(1, &vposition_CP_ID);
    glBindBuffer(GL_ARRAY_BUFFER, vposition_CP_ID);

    // Create a vertex array object  for CURVE
    glGenVertexArrays(1, &vao_2);
    glBindVertexArray(vao_2);
    // attribute VBO : positions 
    glGenBuffers(1, &vposition_Curve_ID);
    glBindBuffer(GL_ARRAY_BUFFER, vposition_Curve_ID);

    // Background color
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glViewport(0, 0, width, height);
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

void deCasteljau(float t, float* result) {
    float coordX[MaxNumPts], coordY[MaxNumPts];

    for (int i = 0; i < NumPts; i++) {
        coordX[i] = vPositions_CP[i][0];
        coordY[i] = vPositions_CP[i][1];
    }

    for (int i = 1; i < NumPts; i++) {
        for (int k = 0; k < NumPts - i; k++) {
            coordX[k] = (1 - t) * coordX[k] + (t)*coordX[k + 1];
            coordY[k] = (1 - t) * coordY[k] + (t)*coordY[k + 1];
        }
    }

    result[0] = coordX[0];
    result[1] = coordY[0];
}

// Funzione per costruire i punti di controllo per curve di Bézier a tratti cubiche
void buildBezierControlPoints() {
    if (NumPts < 4) {
        NumBezierPts = 0;
        return;
    }

    NumBezierPts = 0;

    // Calcoliamo le tangenti m_i per ogni punto interpolato
    // I punti interpolati sono P_0, P_3, P_6, P_9, ... (ogni 3 punti a partire dal primo)

    for (int i = 0; i < NumPts && NumBezierPts < MaxNumPts - 3; i += 3) {
        // Punto corrente da interpolare
        float P_i[2] = { vPositions_CP[i][0], vPositions_CP[i][1] };
        // Calcolo della tangente m_i = (P_{i+1} - P_{i-1}) / 2
        float m_i[2];

        if (i == 0) {
            // Primo punto: usiamo differenza in avanti
            if (i + 1 < NumPts) {
                m_i[0] = (vPositions_CP[i + 1][0] - P_i[0]) * 0.5f;
                m_i[1] = (vPositions_CP[i + 1][1] - P_i[1]) * 0.5f;
            }
        }
        else if (i >= NumPts - 1) {
            // Ultimo punto: usiamo differenza all'indietro
            m_i[0] = (P_i[0] - vPositions_CP[i - 1][0]) * 0.5f;
            m_i[1] = (P_i[1] - vPositions_CP[i - 1][1]) * 0.5f;
        }
        else {
            // Punto intermedio: differenza centrata
            m_i[0] = (vPositions_CP[i + 1][0] - vPositions_CP[i - 1][0]) * 0.5f;
            m_i[1] = (vPositions_CP[i + 1][1] - vPositions_CP[i - 1][1]) * 0.5f;
        }

        // Se non è il primo segmento, aggiungiamo P-_i (punto di controllo entrante)
        if (i > 0 && NumBezierPts < MaxNumPts) {
            float Pminus_i[2] = { P_i[0] - m_i[0] / 3.0f, P_i[1] - m_i[1] / 3.0f };
            vPositions_Bezier[NumBezierPts][0] = Pminus_i[0];
            vPositions_Bezier[NumBezierPts][1] = Pminus_i[1];
            NumBezierPts++;
        }
        // Aggiungiamo il punto interpolato P_i
        if (NumBezierPts < MaxNumPts) {
            vPositions_Bezier[NumBezierPts][0] = P_i[0];
            vPositions_Bezier[NumBezierPts][1] = P_i[1];
            NumBezierPts++;
        }

        // Se non è l'ultimo segmento, aggiungiamo P+_i (punto di controllo uscente)
        if (i < NumPts - 3 && NumBezierPts < MaxNumPts) {
            float Pplus_i[2] = { P_i[0] + m_i[0] / 3.0f, P_i[1] + m_i[1] / 3.0f };
            vPositions_Bezier[NumBezierPts][0] = Pplus_i[0];
            vPositions_Bezier[NumBezierPts][1] = Pplus_i[1];
            NumBezierPts++;
        }
    }
}

// Funzione modificata per usare i punti di controllo Bézier con deCasteljau
void computeBezierCurve() {
    if (NumBezierPts < 4) return;

    int curveIndex = 0;
    int numSegments = (NumBezierPts - 1) / 3;

    // Calcola punti per segmento in base al numero totale di segmenti
    // per non superare MaxNumPts
    int pointsPerSegment = (MaxNumPts- 10) / numSegments; // -10 per sicurezza
    if (pointsPerSegment < 10) pointsPerSegment = 10; // minimo 10 punti per segmento
    if (pointsPerSegment > 50) pointsPerSegment = 50; // massimo 50 punti per segmento

    // Per ogni tratto di curva cubica (4 punti di controllo alla volta)
    for (int tratto = 0; tratto < numSegments && curveIndex < MaxNumPts - pointsPerSegment; tratto++) {
        int startIdx = tratto * 3;

        // Copiamo i 4 punti di controllo del segmento corrente in vPositions_CP temporaneamente
        float tempCP[4][2];
        for (int i = 0; i < 4 && startIdx + i < NumBezierPts; i++) {
            tempCP[i][0] = vPositions_Bezier[startIdx + i][0];
            tempCP[i][1] = vPositions_Bezier[startIdx + i][1];
        }

        // Salviamo i punti originali
        float originalCP[MaxNumPts][2];
        int originalNumPts = NumPts;
        for (int i = 0; i < NumPts; i++) {
            originalCP[i][0] = vPositions_CP[i][0];
            originalCP[i][1] = vPositions_CP[i][1];
        }

        // Impostiamo temporaneamente i 4 punti di controllo per deCasteljau
        NumPts = 4;
        for (int i = 0; i < 4; i++) {
            vPositions_CP[i][0] = tempCP[i][0];
            vPositions_CP[i][1] = tempCP[i][1];
        }

        // Generiamo punti sulla curva usando deCasteljau 
        int actualPointsThisSegment = (tratto == numSegments - 1) ? pointsPerSegment + 1 : pointsPerSegment;

        for (int i = 0; i < actualPointsThisSegment && curveIndex < MaxNumPts; i++) {
            float t = (float)i / (float)(pointsPerSegment);
            if (t > 1.0f) t = 1.0f; // clamp a 1.0

            float result[2];
            deCasteljau(t, result);
            vPositions_C[curveIndex][0] = result[0];
            vPositions_C[curveIndex][1] = result[1];
            curveIndex++;
            if (NumPts >= MaxNumPts - 10) {
                removeFirstPoint();
            }
        }

        // Recuperiamo i punti originali
        NumPts = originalNumPts;
        for (int i = 0; i < NumPts; i++) {
            vPositions_CP[i][0] = originalCP[i][0];
            vPositions_CP[i][1] = originalCP[i][1];
        }
    }

    NumPoints = curveIndex;
    printf("Generati %d punti per la curva con %d punti di controllo\n", NumBezierPts, NumPts);
}

void drawScene(void)
{
    // Background color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0, 0.0, 0.0, 1.0);

    if (NumPts > 3) {
        // Costruiamo i punti di controllo per le curve di Bézier a tratti
        buildBezierControlPoints();

        // Calcoliamo la curva usando deCasteljau sui segmenti
        computeBezierCurve();

        // Disegniamo la curva
        if (NumPoints > 0) {
            glBindVertexArray(vao_2);
            glBindBuffer(GL_ARRAY_BUFFER, vposition_Curve_ID);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vPositions_C), &vPositions_C[0], GL_STREAM_DRAW);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glPointSize(0.5);
            glDrawArrays(GL_POINTS, 0, NumPoints);
            glLineWidth(2.0);
            glDrawArrays(GL_LINE_STRIP, 0, NumPoints);
            glBindVertexArray(0);
        }
    }

    // Draw control polygon
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vposition_CP_ID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vPositions_CP), &vPositions_CP[0], GL_STREAM_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Draw the control points CP
    glPointSize(6.0);
    glDrawArrays(GL_POINTS, 0, NumPts);
    // Draw the line segments between CP
    glLineWidth(2.0);
    glDrawArrays(GL_LINE_STRIP, 0, NumPts);
    glBindVertexArray(0);
}


// Function to initialize OpenGL and create a window.
GLFWwindow* initOpenGL() {

    // Initialize GLFW.
    /*** Inizializzazioni GLFW library *****/
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW." << std::endl;
        glfwTerminate();
    }

    //Imposta le proprietà del contesto e del profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //Abilita il double buffering
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

    /*** Create a window and its OpenGL context ***/
    GLFWwindow* window = glfwCreateWindow(width, height, "LAB_01", NULL, NULL);
    if (!window)
    {
        std::cout << "Failed to create GLFW window !" << std::endl;
        glfwTerminate();
    }
    /* Make the window's context current */
    glfwMakeContextCurrent(window); //crea il context corrente e lo associa a window. In opengl un rendering context è una macchina a stati che memorizza tutte le informazioni necessarie e le risorse per il rendering grafico

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

    /*** Funzioni callback chiamate quando si verificano determinati eventi **/
    //Chiusura dell'applicazione premendo il tasto Esc  e la modifica del colore dello sfondo con space bar
    glfwSetKeyCallback(window, key_callback);
    //Visualizza le coordinate del mouse che si muove sulla finestra grafica
    glfwSetCursorPosCallback(window, cursor_position_callback);
    //Visualizza le coordinate individuate dal tasto sinistro premuto
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    //Visualizza le dimensioni della finestra se ridimensionata
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        // Render 
        drawScene();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);
        /* Poll for and process events */
        glfwPollEvents();
    }

    glDeleteProgram(programId);
    glDeleteVertexArrays(1, &vao);
    glDeleteVertexArrays(1, &vao_2);

    glfwTerminate(); //Libera le risorse allocate da glfwInit
    return 0;
}