#ifndef JUEGOTIPOS_H
#define JUEGOTIPOS_H

#include <vector>
#include <pthread.h>

// Medidas principales del tablero y su posicion en la ventana
const int TableroTamano = 8;
const int CeldaTamano = 60;   
const int DesplazamientoX = 160;   
const int DesplazamientoY = 100;  

// Estados que puede tener el juego segun la pantalla actual
enum EstadoPantalla { 
    Menu, 
    Instrucciones, 
    Modalidad, 
    PantallaCarga, 
    TableroJuego, 
    Pausa, 
    PartidaTerminada 
};

// Modos de juego disponibles
enum ModoJuego { 
    Lento, 
    Rapido, 
    Caos 
};

// Posicion actual del cursor dentro del tablero
struct Cursor { 
    int X = 0; 
    int Y = 0; 
};

// Representa una posicion o punto dentro del tablero
struct Punto { 
    int X; 
    int Y; 
};

// Datos que se mandan al hilo que procesa una jugada
struct DatosJugada {
    std::vector<std::vector<int>>* Tablero;
    std::vector<Punto> CadenaSeleccionada;
    int* Puntuacion;
    int* Movimientos;
    EstadoPantalla* EstadoActual;
    pthread_mutex_t* MutexTablero;
    int* CantidadColores;
    ModoJuego ModoActivo; 
    int MultiplicadorEntero; 
};

// Datos que se mandan al hilo que genera o modifica el tablero
struct DatosTablero {
    std::vector<std::vector<int>>* Tablero;
    pthread_mutex_t* MutexTablero;
    int CantidadColores;
};

#endif