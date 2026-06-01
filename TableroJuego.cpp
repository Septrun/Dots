#include "TableroJuego.h"
#include <cstdlib>
#include <cmath>

// comprueba si un punto especifico ya forma parte de la cadena actual
bool EsPuntoEnCadena(int X, int Y, const std::vector<Punto>& Cadena) {
    for (const auto& Pt : Cadena) {
        if (Pt.X == X && Pt.Y == Y) return true;
    }
    return false;
}

// evalua si el camino seleccionado cierra un ciclo para aplicar bonificaciones
bool FormaCicloCerrado(const std::vector<Punto>& Cadena) {
    if (Cadena.size() < 4) return false;
    Punto Primero = Cadena.front();
    Punto Ultimo = Cadena.back();
    if ((std::abs(Primero.X - Ultimo.X) + std::abs(Primero.Y - Ultimo.Y)) == 1) {
        return true;
    }
    return false;
}

// inicializa el gestor y el mutex para acceso seguro a la matriz
TableroJuegoManager::TableroJuegoManager() : Tablero(TableroTamano, std::vector<int>(TableroTamano)) {
    pthread_mutex_init(&MutexTablero, NULL);
}

// libera recursos del mutex al destruir la instancia
TableroJuegoManager::~TableroJuegoManager() {
    pthread_mutex_destroy(&MutexTablero);
}

// rutina de hilo para generar el estado inicial del tablero segun el modo
void* TableroJuegoManager::HiloGenerarTablero(void* Args) {
    DatosTablero* Datos = (DatosTablero*)Args;
    pthread_mutex_lock(Datos->MutexTablero);
    
    int CodigoModo = Datos->CantidadColores;
    int ColoresReales = CodigoModo % 100;

    for (int I = 0; I < TableroTamano; ++I) {
        for (int J = 0; J < TableroTamano; ++J) {
            if (CodigoModo == 206) { 
                int Probabilidad = rand() % 100;
                if (Probabilidad < 10) {        
                    (*Datos->Tablero)[I][J] = 8;
                } else if (Probabilidad < 13) {  
                    (*Datos->Tablero)[I][J] = 7;
                } else {
                    (*Datos->Tablero)[I][J] = (rand() % ColoresReales) + 1;
                }
            } 
            else {
                (*Datos->Tablero)[I][J] = (rand() % ColoresReales) + 1;
            }
        }
    }
    
    Datos->CantidadColores = ColoresReales;

    pthread_mutex_unlock(Datos->MutexTablero);
    delete Datos; 
    pthread_exit(NULL);
}

// ejecuta la eliminacion de celdas, calculo de puntuacion y gravedad
void* TableroJuegoManager::HiloProcesarJugada(void* Args) {
    DatosJugada* Datos = (DatosJugada*)Args;
    
    pthread_mutex_lock(Datos->MutexTablero);

    bool HayCiclo = FormaCicloCerrado(Datos->CadenaSeleccionada);
    int PuntosEliminados = 0;
    int PuntosBase = 0;

    // procesa unicamente los puntos explicitos de la cadena
    for (const auto& Pt : Datos->CadenaSeleccionada) {
        if ((*Datos->Tablero)[Pt.Y][Pt.X] != 0) {
            (*Datos->Tablero)[Pt.Y][Pt.X] = 0;
            PuntosEliminados++;
        }
    }

    PuntosBase = (HayCiclo) ? (PuntosEliminados * 8) : (PuntosEliminados * 5);

    if (Datos->ModoActivo == Caos || Datos->ModoActivo == Rapido) {
        *Datos->Puntuacion += (PuntosBase * Datos->MultiplicadorEntero) / 10;
    } else {
        *Datos->Puntuacion += PuntosBase;
    }

    (*Datos->Movimientos)--;

    // aplica gravedad y rellenado dinamico por columnas
    for (int X = 0; X < TableroTamano; ++X) {
        int PosicionVacia = TableroTamano - 1;
        for (int Y = TableroTamano - 1; Y >= 0; --Y) {
            if ((*Datos->Tablero)[Y][X] != 0) {
                if (Y != PosicionVacia) {
                    (*Datos->Tablero)[PosicionVacia][X] = (*Datos->Tablero)[Y][X];
                    (*Datos->Tablero)[Y][X] = 0;
                }
                PosicionVacia--;
            }
        }
        
        for (int Y = PosicionVacia; Y >= 0; --Y) {
            if (Datos->ModoActivo == Caos) {
                int Probabilidad = rand() % 100;
                if (Probabilidad < 10)       (*Datos->Tablero)[Y][X] = 8; 
                else if (Probabilidad < 13)  (*Datos->Tablero)[Y][X] = 7; 
                else                         (*Datos->Tablero)[Y][X] = (rand() % (*Datos->CantidadColores)) + 1;
            }
            else {
                (*Datos->Tablero)[Y][X] = (rand() % (*Datos->CantidadColores)) + 1;
            }
        }
    }

    pthread_mutex_unlock(Datos->MutexTablero);
    delete Datos; 
    pthread_exit(NULL);
}

// crea hilo para inicializacion sincronica del tablero
void TableroJuegoManager::InicializarTableroAsync(int CantidadColores) {
    pthread_t Hilo;
    DatosTablero* Argumentos = new DatosTablero{&Tablero, &MutexTablero, CantidadColores};
    pthread_create(&Hilo, NULL, HiloGenerarTablero, (void*)Argumentos);
    pthread_join(Hilo, NULL); 
}

// crea hilo para procesamiento asincronico de jugadas
void TableroJuegoManager::ProcesarJugadaAsync(const std::vector<Punto>& Cadena, int& Puntuacion, int& Movimientos, EstadoPantalla& Estado, int& CantidadColores, ModoJuego Modo, int Multiplicador) {
    pthread_t HiloJugada;
    DatosJugada* ArgumentosJugada = new DatosJugada{&Tablero, Cadena, &Puntuacion, &Movimientos, &Estado, &MutexTablero, &CantidadColores, Modo, Multiplicador};
    pthread_create(&HiloJugada, NULL, HiloProcesarJugada, (void*)ArgumentosJugada);
    pthread_detach(HiloJugada); 
}

// retorno seguro del color de celda mediante mutex
int TableroJuegoManager::ObtenerColorCelda(int X, int Y) {
    pthread_mutex_lock(&MutexTablero);
    int Color = Tablero[Y][X];
    pthread_mutex_unlock(&MutexTablero);
    return Color;
}

// genera copia del estado del tablero para renderizado
void TableroJuegoManager::ClonarTablero(int Destino[TableroTamano][TableroTamano]) {
    pthread_mutex_lock(&MutexTablero);
    for (int I = 0; I < TableroTamano; ++I) {
        for (int J = 0; J < TableroTamano; ++J) {
            Destino[I][J] = Tablero[I][J];
        }
    }
    pthread_mutex_unlock(&MutexTablero); 
}

// valida conexion logica entre celdas incluyendo manejo de obstaculos y comodines
bool TableroJuegoManager::ValidarConexion(Punto Ultimo, Cursor Actual) {
    pthread_mutex_lock(&MutexTablero);
    int ColorUltimo = Tablero[Ultimo.Y][Ultimo.X];
    int ColorActual = Tablero[Actual.Y][Actual.X];
    pthread_mutex_unlock(&MutexTablero);

    if (ColorUltimo == 8 || ColorActual == 8) return false;

    if (ColorUltimo == ColorActual || ColorUltimo == 7 || ColorActual == 7) {
        if ((std::abs(Ultimo.X - Actual.X) + std::abs(Ultimo.Y - Actual.Y)) == 1) {
            return true;
        }
    }
    return false;
}