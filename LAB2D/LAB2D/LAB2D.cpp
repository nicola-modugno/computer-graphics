/* LAB_2_2D.cpp :
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
#include "imgui\imgui.h"
#include "imgui\imgui_impl_glfw.h"
#include "imgui\imgui_impl_opengl3.h"
#include "Gui.h"
#include <math.h>


#define PI 3.14159265358979323846
#define _CRT_SECURE_NO_WARNINGS
bool fireworks_activated = false;
static double	limitFPS = 1.0 / 60.0;

typedef struct { float x, y, r, g, b, a; } Pointxy;

float ombra_posy = 0;
void disegna_pianoxy(float x, float y, float width, float height, vec4 color_top, vec4 color_bot, Pointxy* piano);
void disegna_PalaEolica(int nTriangles, Pointxy* PalaEolica);
void disegna_palla(int nTriangles, Pointxy* Palla);
void disegna_sole(int nTriangles, Pointxy* Sole);
void disegna_montagne(float x0, float y0, int altezza_montagna, int larghezza_montagne, int numero_di_montagne, Pointxy* v_montagna);

// viewport size
int     width = 1200;
int     height = 700;
mat4    Projection;  //Matrice di proiezione
mat4    Model;       //Matrice di modellazione


vec4 lerpColor(vec4 a, vec4 b, float t) {
    return vec4{
        (1 - t) * a.r + t * b.r,
        (1 - t) * a.g + t * b.g,
        (1 - t) * a.b + t * b.b,
        (1 - t) * a.a + t * b.a
    };
}
// Variabili per il gameplay------------------------------------------------------------
float game_timer = 8.0; // 8 secondi iniziali
bool game_over = false;
int punti;


//float penalita_base = 1.0f / 60.0f; //fattore per calcolare il tempo rimanente
float difficolta = 1.0f;


vec2 sbarra_pos = vec2(0.0f);   // posizione della sbarra attiva
float sbarra_larghezza = 100.0f;
float sbarra_altezza = 20.0f;

bool sbarra_visibile = false;  // serve a sapere quando mostrarla

float tempo_tra_sbarre = 0.0f;
bool prima_sbarra = true;
// -------------------------------------------------------------------------------------

void generaNuovaSbarra() {
    if (prima_sbarra) {
        punti = 0;
        prima_sbarra = false;
    }
    else {
        punti += 10;
        difficolta += 0.6f;
    }
    game_timer += 0.6f / 60;  // aggiungo 1 secondo ogni volta che viene generata una nuova sbarra.
    float margine = 50.0f;
    sbarra_pos.x = margine + static_cast<float>(rand()) / RAND_MAX * (width - 2 * margine - sbarra_larghezza);
    sbarra_pos.y = height * 0.25f + static_cast<float>(rand()) / RAND_MAX * (height * 0.25f);
    sbarra_visibile = true;
}


// Variabili globali -------------------------------------------------------------------
static unsigned int programId;

// Oggetti - Geometria
unsigned int VAO_MONTAGNE, VAO_PRATO, VAO_SOLE, VAO_CIELO, VAO_PALLA, VAO_PalaEolica, VAO_PARTICELLE, VAO_SBARRA;
unsigned int VBO_M, VBO_P, VBO_S, VBO_C, VBO_Pa, VBO_PE, VBO_PARTICELLE, VBO_SBARRA, MatProj, MatModel;

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
extern vec4 col_red;
extern Pointxy* Sbarra;
extern int vertices_Sbarra;


// Variabili sistema particellare 
extern int n_Particellari;
extern Pointxy* Particelle;
extern int p_attivi = 0;
int contatore_esplosione = 0;
const int intervallo_esplosione = 60; // ogni 60 frame (1 sec circa)


// parametri della palla
float	distacco_da_terra = 0;	    // distacco da terra 
float distanza_dal_prato;
float velocitaY = 0.0f;  // velocità verticale
float gravita = -1.2f;
float	delta = 15.0;
float	Muovi_Orizzontale = 0.0;    // velocita orizzontale (pixel per frame)
float	posx = float(width) / 2.0f; // posizione iniziale della palla
float	posy = float(height) * 0.2f;

bool    pressing_left = false;
bool    pressing_right = false;
bool    mouse_pressed = false;
//posizione del puntatore del mouse
extern float   mouse_pointer_x, mouse_pointer_y;

float tempo_sole = 0.00;


// Definizione arco del sole --------------------------
// Centro dell’arco in alto
float cx = width / 2.0f;
float cy = height * 0.4f;

// Raggi orizzontale e verticale dell’arco
float rx = width * 0.6f;
float ry = height * 0.12f;

// Parametro angolare da 0 a 2? (un ciclo completo)
float t = fmod(tempo_sole, 2 * PI);

// Posizione del sole lungo l’arco
float xSole = cx + rx * cos(t + PI);
float ySole = cy + ry * sin(t);
//-----------------------------------------------------

//Colore cielo
float fase_sole = 0;
vec4 cielo_giorno_top = { 0.3, 0.6, 1.0, 1.0 };
vec4 cielo_giorno_bot = { 0.0, 0.1, 1.0, 1.0 };
vec4 cielo_notte_top = { 0.05, 0.05, 0.2, 1.0 };
vec4 cielo_notte_bot = { 0.0, 0.0, 0.1, 1.0 };

vec4 alone_giorno = { 1.0, 0.8, 0.0, 0.7 };
vec4 alone_notte = { 1.0, 0.5, 0.0, 0.2 };

vec4 col_top = lerpColor(cielo_notte_top, cielo_giorno_top, fase_sole);
vec4 col_bot = lerpColor(cielo_notte_bot, cielo_giorno_bot, fase_sole);


vec3 pos_pala_eolica1, pos_pala_eolica2, pos_pala_eolica3, pos_pala_eolica4, pos_pala_eolica5, pos_pala_eolica6;

//Interpolazione lineare tra a e b con parametro amount
float lerp(float a, float b, float amount) {
    return (1 - amount) * a + amount * b;
}

/*
Gestione Sistema Particellare
//////////////////////////////////////////////////////////////////////////////
*/
void rimuoviPoiScorri(int index) {
    int i;
    p_attivi--;
    if (index != p_attivi) {
        // Remove the first point, slide the rest down
        for (i = index; i < p_attivi; i++) {
            Particelle[i] = Particelle[i + 1];
        }
    }
}

void aggiungiParticella(vec3 posizione, vec3 direzione) {

    //Eliminiamo particelle oltre il massimo consentito
    while (p_attivi >= n_Particellari)
        rimuoviPoiScorri(0);

    int i = p_attivi;
    Particelle[i].x = (float)posizione.x;
    Particelle[i].y = (float)posizione.y;
    Particelle[i].r = ((float)direzione.x + 1) / 2;
    Particelle[i].g = ((float)direzione.y + 1) / 2;
    Particelle[i].b = 0.3f;
    Particelle[i].a = 1.0f;
    p_attivi++;
}

void aggiungiFlussoColorato(vec3 position) {
    vec3 posizione = position;
    vec3 direzione_start = vec3{ 0,0,0 };
    vec3 direzione = direzione_start;
    float angolo;
    float step = PI / 32;
    for (angolo = 0; angolo < (2 * PI) - step; angolo += step) {
        direzione.x = glm::cos(angolo);
        direzione.y = glm::sin(angolo);
        aggiungiParticella(posizione, direzione);
    }
}

void muoviParticella(int index) {

    //Calcolo "direzione"
    float r = Particelle[index].r;
    float g = Particelle[index].g;
    float dx = (r * 2) - 1;
    float dy = (g * 2) - 1;

    //Aggiorno posizione e alfa-value
    Particelle[index].x += dx;
    Particelle[index].y += dy;
    Particelle[index].a -= 0.005f;

    //printf("n:%d\tx:%f\ty:%f\tr:%f\tg:%f\n", index, Particelle[index].x, Particelle[index].y, Particelle[index].r, Particelle[index].g);
}

void aggiornaParticelle() {
    int i;
    for (i = 0; i < p_attivi; i++) {
        muoviParticella(i);
        if (Particelle[i].a <= 0)
            rimuoviPoiScorri(i);
    }
}
//////////////////////////////////////////////////////////////////////////////

/************************************************************/
void init(void)
{
    Projection = ortho(0.0f, float(width), 0.0f, float(height));
    MatProj = glGetUniformLocation(programId, "Projection");
    MatModel = glGetUniformLocation(programId, "Model");


    //Gestione trasparenza : canale colore alpha 
    glEnable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    //glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //Costruzione geometria e colori del CIELO
    vec4 col_top = { 0.3,0.6,1.0,1.0 };
    vec4 col_bottom = { 0.0 , 0.1, 1.0, 1.0 };
    disegna_pianoxy(0, height * 0.5f, (GLfloat)width, height * 0.5f, col_bottom, col_top, Cielo);
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
    disegna_pianoxy(0.0f, 0.0f, (GLfloat)width, (GLfloat)height * 0.5f, col_bottom, col_top, Prato);
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

    //Genero VAO e VBO dei particellari
    glGenVertexArrays(1, &VAO_PARTICELLE);
    glBindVertexArray(VAO_PARTICELLE);
    glGenBuffers(1, &VBO_PARTICELLE);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_PARTICELLE);
    glBufferData(GL_ARRAY_BUFFER, n_Particellari * sizeof(Pointxy), &Particelle[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    //Disegno la sbarra
    disegna_pianoxy(0.0f, 0.0f, 1.0f, 1.0f, col_red, col_red, Sbarra); // rettangolo base 1x1, sarà scalato
    glGenVertexArrays(1, &VAO_SBARRA);
    glBindVertexArray(VAO_SBARRA);

    glGenBuffers(1, &VBO_SBARRA);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_SBARRA);
    glBufferData(GL_ARRAY_BUFFER, vertices_Sbarra * sizeof(Pointxy), Sbarra, GL_STATIC_DRAW);

    // layout shader
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Pointxy), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Pointxy), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // Background color
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glViewport(0, 0, width, height);
}

//Movimento della palla in orizzontale
void update()
{
    tempo_sole += 0.01;
    bool moving = false;
    if (game_over) {
        if (contatore_esplosione >= intervallo_esplosione) {
            aggiungiFlussoColorato(pos_pala_eolica1);
            aggiungiFlussoColorato(pos_pala_eolica2);
            aggiungiFlussoColorato(pos_pala_eolica3);
            aggiungiFlussoColorato(pos_pala_eolica4);
            aggiungiFlussoColorato(pos_pala_eolica5);
            aggiungiFlussoColorato(pos_pala_eolica6);
            contatore_esplosione = 0; // reset
        }
        else {
            contatore_esplosione++;
        }
    }

    if (!game_over) {

        game_timer -= difficolta / 60;


        if (game_timer <= 0.0f) {
            game_timer = 0.0f;
            game_over = true;
        }
    }

    if (p_attivi > 0) {
        aggiornaParticelle();
    }


    if (mouse_pressed) {
        return;
    }

    if (sbarra_visibile && !mouse_pressed) {
        float pallax = posx;
        float pallay = posy;

        // controlla se la palla tocca la sbarra
        if (pallax + 40 > sbarra_pos.x && pallax - 40 < sbarra_pos.x + sbarra_larghezza &&
            pallay <= sbarra_pos.y + sbarra_altezza &&
            pallay >= sbarra_pos.y - 10) // margine tolleranza verticale
        {
            // COLPITO!
            sbarra_visibile = false;
            game_timer += 5.0f; // bonus tempo
            generaNuovaSbarra();
        }
    }


    if (pressing_left) {
        Muovi_Orizzontale -= 1;
        moving = true;
    }
    if (pressing_right) {
        Muovi_Orizzontale += 1;
        moving = true;
    }

    posx += Muovi_Orizzontale;

    if (posx < 0.0) {
        posx = 0.0f;
        Muovi_Orizzontale *= -0.8f;
    }
    if (posx > width) {
        posx = (float)width;
        Muovi_Orizzontale *= -0.8f;
    }

    // Fisica verticale (gravità + rimbalzo reale)
    velocitaY += gravita;
    posy += velocitaY;

    float groundLevel = height * 0.2f; // livello del prato, da regolare

    if (posy <= groundLevel) {
        posy = groundLevel;
        velocitaY *= -0.8f;  // rimbalzo attenuato

        // se il rimbalzo è troppo piccolo, ferma tutto
        if (fabs(velocitaY) < 1.0f)
            velocitaY = 0;
    }

    // Limita l’altezza massima se necessario
    if (posy > height - 80) {
        posy = height - 80;
        velocitaY *= -0.5f;
    }
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
float palla_posx, palla_posy;

void drawScene(void)
{
    //Background color
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

    // --- SOLE IN MOVIMENTO AD ARCO E CIELO DINAMICO ---

    // Parametri arco
    float t = fmod(tempo_sole, 2 * PI);
    float cx = width / 2.0f;
    float cy = height * 0.5f;
    float rx = width * 0.6f;
    float ry = height * 0.4f;
    float xSole = cx + rx * cos(t + PI);
    float ySole = cy + ry * sin(t);

    // Fase giorno-notte: da 0 (notte) a 1 (giorno) 
    float fase_sole = clamp(sin(t), 0.0f, 1.0f);

    // Cielo dinamico
    vec4 col_top = lerpColor(cielo_notte_top, cielo_giorno_top, fase_sole);
    vec4 col_bot = lerpColor(cielo_notte_bot, cielo_giorno_bot, fase_sole);
    disegna_pianoxy(0, height * 0.5f, (GLfloat)width, height * 0.5f, col_bot, col_top, Cielo);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_C);
    glBufferData(GL_ARRAY_BUFFER, vertices_Cielo * sizeof(Pointxy), &Cielo[0], GL_STATIC_DRAW);
    Model = mat4(1.0);
    glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
    glBindVertexArray(VAO_CIELO);
    glDrawArrays(GL_TRIANGLES, 0, vertices_Cielo);
    glBindVertexArray(0);

    // --- SOLE e ALONE
    if (ySole >= height * 0.5f) {
        vec4 colore_alone = lerpColor(alone_notte, alone_giorno, fase_sole);

        for (int i = vertices_sole / 2; i < vertices_sole; i++) {
            Sole[i].r = colore_alone.r;
            Sole[i].g = colore_alone.g;
            Sole[i].b = colore_alone.b;
            Sole[i].a = colore_alone.a;
        }
        glBindBuffer(GL_ARRAY_BUFFER, VBO_S);
        glBufferData(GL_ARRAY_BUFFER, vertices_sole * sizeof(Pointxy), &Sole[0], GL_STATIC_DRAW);

        // Sole centrale
        Model = mat4(1.0);
        Model = translate(Model, vec3(xSole, ySole, 0.0));
        Model = scale(Model, vec3(30.0, 30.0, 1.0));
        glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
        glBindVertexArray(VAO_SOLE);
        glDrawArrays(GL_TRIANGLES, 0, vertices_sole / 2);

        // Alone del sole
        Model = mat4(1.0);
        Model = translate(Model, vec3(xSole, ySole, 0.0));
        Model = scale(Model, vec3(80.0, 80.0, 1.0));
        glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
        glDrawArrays(GL_TRIANGLES, vertices_sole / 2, vertices_sole / 2);
        glBindVertexArray(0);
    }

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
    float maxHeight = height * 0.8f;
    float minHeight = height * 0.2f;
    float dist = (posy - minHeight);
    float maxDist = maxHeight - minHeight;

    float shadow_scale = lerp(1.0f, 0.2f, dist / maxDist);
    float bwidth = 80.0f;
    float bheight = 80.0f;
    //Matrice per il cambiamento di posizione dell'OMBRA della palla
    //std::cout << posy + 10 + 10 * (1 - shadow_scale) << std::endl;
    if (posy + 10 + 10 * (1 - shadow_scale) >= 331) {
        Model = translate(Model, vec3(posx - bwidth / 2 * shadow_scale, 330, 0.0f));
        ombra_posy = 330;
    }
    else {
        Model = translate(Model, vec3(posx - bwidth / 2 * shadow_scale, posy + 10 + 10 + 10 + 10 * (1 - shadow_scale), 0.0f));
        ombra_posy = posy + 10 + 10 + 10 + 10 * (1 - shadow_scale);
    }
    Model = scale(Model, vec3(float(bwidth) * shadow_scale, (50.0 * shadow_scale), 1.0));
    glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
    glBindVertexArray(VAO_PALLA);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawArrays(GL_TRIANGLES, vertices_palla / 2, vertices_palla / 2);

    //Matrice per il cambiamento di posizione della PALLA
    Model = mat4(1.0);
    Model = translate(Model, vec3(posx - bwidth / 2, posy + bheight + distacco_da_terra, 0.0f));
    palla_posx = posx - bwidth / 2;
    palla_posy = posy + bheight + distacco_da_terra;
    Model = scale(Model, vec3(float(bwidth) / 2, float(bheight) / 2, 1.0));
    glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawArrays(GL_TRIANGLES, 0, vertices_palla / 2);
    glBindVertexArray(0);

    //Particellari
    Model = mat4(1.0);
    glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
    glGenVertexArrays(1, &VAO_PARTICELLE);
    glBindVertexArray(VAO_PARTICELLE);
    glGenBuffers(1, &VBO_PARTICELLE);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_PARTICELLE);
    glBufferData(GL_ARRAY_BUFFER, n_Particellari * sizeof(Pointxy), &Particelle[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glPointSize(3.0);
    glDrawArrays(GL_POINTS, 0, p_attivi);
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
        float centroX = float(width) * 0.15f * i;
        float centroY = float(height) * 0.62f;
        float lunghezza_pala = 40.0f;
        float angolo_rad = radians(angolo);

        // Posizione della punta della pala
        float tipX = centroX + lunghezza_pala * cos(angolo_rad);
        float tipY = centroY + lunghezza_pala * sin(angolo_rad);

        switch (i) {
        case 1:
            pos_pala_eolica1 = vec3(tipX, tipY, 0.0f);
            break;
        case 2:
            pos_pala_eolica2 = vec3(tipX, tipY, 0.0f);
            break;
        case 3:
            pos_pala_eolica3 = vec3(tipX, tipY, 0.0f);
            break;
        case 4:
            pos_pala_eolica4 = vec3(tipX, tipY, 0.0f);
            break;
        case 5:
            pos_pala_eolica5 = vec3(tipX, tipY, 0.0f);
            break;
        case 6:
            pos_pala_eolica6 = vec3(tipX, tipY, 0.0f);
            break;
        }

        Model = scale(Model, vec3(40.0, 40.0, 1.0));
        angolo += 0.06;
        Model = rotate(Model, radians(angolo), vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawArrays(GL_TRIANGLES, 0, vertices_PalaEolica - 6); // Pala (Starting from vertex 0)
    }
    //Disegno la sbarra
    if (sbarra_visibile) {
        Model = mat4(1.0);
        Model = translate(Model, vec3(sbarra_pos.x, sbarra_pos.y, 0.0f));
        Model = scale(Model, vec3(sbarra_larghezza, sbarra_altezza, 1.0f));
        glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
        glBindVertexArray(VAO_SBARRA);
        glDrawArrays(GL_TRIANGLES, 0, vertices_Sbarra);
        glBindVertexArray(0);
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
    GLFWwindow* window = glfwCreateWindow(width, height, "LAB_2_JUMP_BALL", NULL, NULL);
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
    generaNuovaSbarra();  // crea la prima sbarra visibile
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

    //Inizializza IMGUI
    init_GUI(window);   //in Gui.cpp

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
        my_interface(window);        

        glfwSwapBuffers(window);

        frames++;

        // Reset after one second
        if (glfwGetTime() - timer > 1.0) {
            timer++;
            std::cout << "FPS: " << frames << " Number of Updates:" << updates << std::endl;
            updates = 0, frames = 0;
        }

        /* Poll for and process events*/
        glfwPollEvents();
    }

    glDeleteProgram(programId);
    glDeleteVertexArrays(1, &VAO_MONTAGNE);
    glDeleteVertexArrays(1, &VAO_PRATO);
    glDeleteVertexArrays(1, &VAO_CIELO);
    glDeleteVertexArrays(1, &VAO_PALLA);
    glDeleteVertexArrays(1, &VAO_PalaEolica);
    glDeleteVertexArrays(1, &VAO_PARTICELLE);
    
    // Chiude il menu di interfaccia con l'utente
    close_GUI();    //in Gui.cpp
    glfwTerminate(); //Libera le risorse allocate da glfwInit
    return 0;
}