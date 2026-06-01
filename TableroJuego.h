#ifndef TABLEROJUEGO_H
#define TABLEROJUEGO_H

#include "JuegoTipos.h"

// Clase encargada de manejar el tablero y la logica principal de las jugadas
class TableroJuegoManager {
private:
    // Matriz donde se guardan los puntos del tablero
    std::vector<std::vector<int>> Tablero;

    // Mutex usado para proteger el tablero cuando se trabaja con hilos
    pthread_mutex_t MutexTablero;

    // Funciones que se ejecutan dentro de los hilos
    static void* HiloGenerarTablero(void* Args);
    static void* HiloProcesarJugada(void* Args);

public:
    // Constructor y destructor de la clase
    TableroJuegoManager();
    ~TableroJuegoManager();

    // Inicializa el tablero usando un hilo
    void InicializarTableroAsync(int CantidadColores); 

    // Procesa la jugada seleccionada por el jugador usando un hilo
    void ProcesarJugadaAsync(const std::vector<Punto>& Cadena, int& Puntuacion, int& Movimientos, EstadoPantalla& Estado, int& CantidadColores, ModoJuego Modo, int Multiplicador);
    
    // Funciones para leer, copiar y validar datos del tablero
    int ObtenerColorCelda(int X, int Y);
    void ClonarTablero(int Destino[TableroTamano][TableroTamano]);
    bool ValidarConexion(Punto Ultimo, Cursor Actual);
};

// Funciones auxiliares para revisar la cadena de puntos seleccionados
bool EsPuntoEnCadena(int X, int Y, const std::vector<Punto>& Cadena);
bool FormaCicloCerrado(const std::vector<Punto>& Cadena);

#endif