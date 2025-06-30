#include "lib.h"

#define PI 3.14159265358979323846

/*-------------COLORI---------------------*/
vec4 col_white = { 1.0,1.0,1.0, 1.0 };
vec4 col_white_trasp = { 1.0, 1.0, 1.0, 0.0 };
vec4 col_red = { 1.0,0.0,0.0, 1.0 };
vec4 col_green = { 0.0,1.0,0.0, 1.0 };
vec4 col_white2 = { 1,1,0.9, 1.0 };
vec4 col_black = { 0.0,0.0,0.0, 1.0 };
vec4 col_bottom_PE = { 0.5, 0.5, 0.05, 0.8 };
vec4 col_orange = { 1.0, 0.8, 0.0, 1.0 };
vec4 col_orange2 = { 1.0, 0.8627, 0.0, 1.0 };
vec4 col_gray_top = { 0.49,0.49,0.49, 0.0 };
vec4 col_black_trasp = { 0.0, 0.0, 0.0, 0.6 };
vec4 col_top_m = { 0.5451, 0.2706, 0.0745, 1.0000 };
vec4 col_bottom_m = { 0.4980, 0.0353, 0.1843, 1.0000 };
/*---------------------------------------*/

// Oggetti - Geometria
typedef struct { float x, y, r, g, b, a; } Pointxy;
float angolo = 0.0;
int Num_strip_instances = 128;
int nVertices_montagna = 2 * Num_strip_instances;
Pointxy* Punti = new Pointxy[nVertices_montagna];
int vertices_Prato = 6;
Pointxy* Prato = new Pointxy[vertices_Prato];
int vertices_Cielo = 6;
Pointxy* Cielo = new Pointxy[vertices_Cielo];
int nTriangles_sole = 30;
int vertices_sole = 3 * 2 * nTriangles_sole;
Pointxy* Sole = new Pointxy[vertices_sole];
int nTriangles_palla = 30;
int vertices_palla = 3 * 2 * nTriangles_palla;
Pointxy* Palla = new Pointxy[vertices_palla];
int nTriangles_PalaEolica = 4;
int vertices_PalaEolica = 3 * (nTriangles_PalaEolica + 2);
Pointxy* PalaEolica = new Pointxy[vertices_PalaEolica];

//----------------------------------------------------------------------------------------
void disegna_pianoxy(float x, float y, float width, float height, vec4 color_top, vec4 color_bot, Pointxy* piano)
{
    piano[0].x = x;	piano[0].y = y;
    piano[0].r = color_bot.r; piano[0].g = color_bot.g; piano[0].b = color_bot.b; piano[0].a = color_bot.a;
    piano[1].x = x + width;	piano[1].y = y;
    piano[1].r = color_top.r; piano[1].g = color_top.g; piano[1].b = color_top.b; piano[1].a = color_top.a;
    piano[2].x = x + width;	piano[2].y = y + height;
    piano[2].r = color_bot.r; piano[2].g = color_bot.g; piano[2].b = color_bot.b; piano[2].a = color_bot.a;

    piano[3].x = x + width;	piano[3].y = y + height;
    piano[3].r = color_bot.r; piano[3].g = color_bot.g; piano[3].b = color_bot.b; piano[3].a = color_bot.a;
    piano[4].x = x;	piano[4].y = y + height;
    piano[4].r = color_top.r; piano[4].g = color_top.g; piano[4].b = color_top.b; piano[4].a = color_top.a;
    piano[5].x = x;	piano[5].y = y;
    piano[5].r = color_bot.r; piano[5].g = color_bot.g; piano[5].b = color_bot.b; piano[5].a = color_bot.a;
}

void disegna_cerchio(int nTriangles, int step, vec4 color_top, vec4 color_bot, Pointxy* Cerchio) {
    int i;
    float stepA = (2.0f * PI) / nTriangles;

    int comp = 0;
    // step = 1 -> triangoli adiacenti, step = n -> triangoli distanti step l'uno dall'altro
    for (i = 0; i < nTriangles; i += step)
    {
        Cerchio[comp].x = cos((float)i * stepA);
        Cerchio[comp].y = sin((float)i * stepA);
        Cerchio[comp].r = color_top.r; Cerchio[comp].g = color_top.g; Cerchio[comp].b = color_top.b; Cerchio[comp].a = color_top.a;

        Cerchio[comp + 1].x = cos((float)(i + 1) * stepA);
        Cerchio[comp + 1].y = sin((float)(i + 1) * stepA);
        Cerchio[comp + 1].r = color_top.r; Cerchio[comp + 1].g = color_top.g; Cerchio[comp + 1].b = color_top.b; Cerchio[comp + 1].a = color_top.a;

        Cerchio[comp + 2].x = 0.0;
        Cerchio[comp + 2].y = 0.0;
        Cerchio[comp + 2].r = color_bot.r; Cerchio[comp + 2].g = color_bot.g; Cerchio[comp + 2].b = color_bot.b; Cerchio[comp + 2].a = color_bot.a;
        comp += 3;
    }
}

void disegna_PalaEolica(int nTriangles, Pointxy* PalaEolica) {

    int i, cont;
    Pointxy* Sostegno;
    int vertici = 6;
    Sostegno = new Pointxy[vertici];
    
    //Costruisco la geometria della pala eolica e il suo sostegno
    disegna_cerchio(nTriangles * 2, 2, col_green, col_bottom_PE, PalaEolica);
    disegna_pianoxy(-0.1f, -1.0f, 1.0f, 1.0f, col_black, col_black, Sostegno);

    // Aggiungo a PalaEolica i vertici dei 2 triangoli del supporto
    cont = 3 * nTriangles;  
    for (i = 0; i < 6; i++)
    {
        PalaEolica[cont + i].x = Sostegno[i].x; PalaEolica[cont + i].y = Sostegno[i].y;
        PalaEolica[cont + i].r = Sostegno[i].r;	PalaEolica[cont + i].g = Sostegno[i].g;	PalaEolica[cont + i].b = Sostegno[i].b; PalaEolica[cont + i].a = Sostegno[i].a;
    }
}

void disegna_palla(int nTriangles, Pointxy* Palla) {

    int      i, cont;
    int      vertici = 3 * nTriangles;
    Pointxy* Ombra;
    Ombra = new Pointxy[vertici];
    
    //Costruisco la geometria della palla ed i suoi colori
    disegna_cerchio(nTriangles, 1, col_red, col_orange, Palla);
    //Costruisco la geometria dell'ombra ed i suoi colori.
    disegna_cerchio(nTriangles, 1, col_gray_top, col_black_trasp, Ombra);

    //Appendo alla Palla la sua Ombra
    cont = 3 * nTriangles;
    for (i = 0; i < 3 * nTriangles; i++)
    {
        Palla[cont + i].x = Ombra[i].x; Palla[cont + i].y = Ombra[i].y;
        Palla[cont + i].r = Ombra[i].r;	Palla[cont + i].g = Ombra[i].g;	Palla[cont + i].b = Ombra[i].b;	Palla[cont + i].a = Ombra[i].a;
    }
}

void disegna_sole(int nTriangles, Pointxy* Sole) {
    int i, cont;
    Pointxy* OutSide;
    int vertici = 3 * nTriangles;
    OutSide = new Pointxy[vertici];

    disegna_cerchio(nTriangles, 1, col_white, col_orange2, Sole);
    disegna_cerchio(nTriangles, 1, col_white_trasp, col_orange2, OutSide);

    cont = 3 * nTriangles;
    for (i = 0; i < 3 * nTriangles; i++)
    {
        Sole[cont + i].x = OutSide[i].x; Sole[cont + i].y = OutSide[i].y;
        Sole[cont + i].r = OutSide[i].r; Sole[cont + i].g = OutSide[i].g; Sole[cont + i].b = OutSide[i].b; Sole[cont + i].a = OutSide[i].a;
    }
}

void disegna_montagne(float x0, float y0, int altezza_montagna, int larghezza_montagne, int numero_montagne, Pointxy* v_montagna)
{
    float length_instance = larghezza_montagne / (float)Num_strip_instances;
    float frequenza = PI * numero_montagne;
    
    int n_vertici = 0;
    // Render: triangle strip
    for (int i = 0; i < Num_strip_instances; i += 1)
    {
        v_montagna[n_vertici].x = x0 + i * length_instance;
        v_montagna[n_vertici].y = y0;
        v_montagna[n_vertici].r = col_bottom_m.r; v_montagna[n_vertici].g = col_bottom_m.g; v_montagna[n_vertici].b = col_bottom_m.b; v_montagna[n_vertici].a = col_bottom_m.a;

        v_montagna[n_vertici + 1].x = x0 + i * length_instance;
        v_montagna[n_vertici + 1].y = y0 + altezza_montagna * abs(sin(i / (float)Num_strip_instances * frequenza));
        v_montagna[n_vertici + 1].r = col_top_m.r; v_montagna[n_vertici + 1].g = col_top_m.g; v_montagna[n_vertici + 1].b = col_top_m.b; v_montagna[n_vertici + 1].a = col_top_m.a;

        n_vertici += 2;
    }
}