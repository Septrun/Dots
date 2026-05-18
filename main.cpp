#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <cmath>
#include <pthread.h>

// Configuración del tamaño del tablero y la GUI
const int TableroTamano = 8;
const int CeldaTamano = 60;   
const int DesplazamientoX = 160;   
const int DesplazamientoY = 100;  

// Pantallas y los modos de dificultad
enum EstadoPantalla { Menu, Modalidad, TableroJuego, PartidaTerminada };
enum ModoJuego { Lento, Rapido };

// Estructura para la posición del cursor y las coordenadas de los cuadros
struct Cursor { int X = 0; int Y = 0; };
struct Punto { int X; int Y; };

// Estructura para el Thread que procesa la jugada seleccionada por el usuario
struct DatosJugada {
    std::vector<std::vector<int>>* Tablero;
    std::vector<Punto> CadenaSeleccionada;
    int* Puntuacion;
    int* Movimientos;
    EstadoPantalla* EstadoActual;
    pthread_mutex_t* MutexTablero;
};

// Estructura para los Threads encargados de reescribir o inicializar el tablero
struct DatosTablero {
    std::vector<std::vector<int>>* Tablero;
    pthread_mutex_t* MutexTablero;
};

// Función para comprobar si una celda específica ya pertenece a la selección actual
bool EsPuntoEnCadena(int X, int Y, const std::vector<Punto>& Cadena) {
    for (const auto& Pt : Cadena) {
        if (Pt.X == X && Pt.Y == Y) return true;
    }
    return false;
}

// Rutina del Hilo 1 para rellenar o resetear todas las celdas de la matriz
void* HiloGenerarTablero(void* Args) {
    DatosTablero* Datos = (DatosTablero*)Args;
    
    // Bloqueo del mutex durante la inicialización global
    pthread_mutex_lock(Datos->MutexTablero);
    for (int I = 0; I < TableroTamano; ++I) {
        for (int J = 0; J < TableroTamano; ++J) {
            (*Datos->Tablero)[I][J] = (rand() % 4) + 1;
        }
    }
    pthread_mutex_unlock(Datos->MutexTablero);
    
    // Liberación de la memoria y salida limpia del hilo
    delete Datos; 
    pthread_exit(NULL);
}

// Rutina de los Hilos 2, 3 y 4 para procesar los puntos eliminados, el puntaje y el final del juego
void* HiloProcesarJugada(void* Args) {
    DatosJugada* Datos = (DatosJugada*)Args;

    // Bloqueo del recurso compartido para realizar mutaciones seguras en el estado del juego
    pthread_mutex_lock(Datos->MutexTablero);

    // Actualización de la puntuación acumulada y reducción de los movimientos disponibles
    *Datos->Puntuacion += Datos->CadenaSeleccionada.size() * 5;
    (*Datos->Movimientos)--;

    // Regeneración en las posiciones de los elementos de la cadena eliminada
    for (const auto& Pt : Datos->CadenaSeleccionada) {
        (*Datos->Tablero)[Pt.Y][Pt.X] = (rand() % 4) + 1;
    }

    // Evaluación del fin de la partida con base en los movimientos restantes
    if (*Datos->Movimientos <= 0) {
        *Datos->EstadoActual = PartidaTerminada;
    }

    pthread_mutex_unlock(Datos->MutexTablero);

    // Liberación de la memoria y cierre del hilo
    delete Datos; 
    pthread_exit(NULL);
}

// Entrada principal: configuración de SFML, ciclo de vida e hilos secundarios
int main() {
    sf::RenderWindow Ventana(sf::VideoMode(800, 600), "Dots Game - Pocket Edition");
    Ventana.setFramerateLimit(60);

    // Inicialización del mutex para proteger el tablero
    pthread_mutex_t MutexTablero;
    pthread_mutex_init(&MutexTablero, NULL);

    // Carga de la fuente del juego
    sf::Font FuenteJuego;
    if (!FuenteJuego.loadFromFile("assets/Baxoe.ttf")) {
        std::cerr << "Error. No se pudo cargar la fuente assets/Baxoe.ttf" << std::endl;
        return -1;
    }

    // Carga de texturas gráficas (assets)
    sf::Texture TCursor, TAzul, TRojo, TVerde, TAmarillo, TLogo;

    if (!TCursor.loadFromFile("assets/Cursor.png") ||
        !TAzul.loadFromFile("assets/Azul.png") ||
        !TRojo.loadFromFile("assets/Rojo.png") ||
        !TVerde.loadFromFile("assets/Verde.png") ||
        !TAmarillo.loadFromFile("assets/Amarillo.png") ||
        !TLogo.loadFromFile("assets/Logo.png")) {
        std::cerr << "Error. No se pudieron cargar las imagenes desde la carpeta assets/" << std::endl;
        return -1;
    }

    sf::Sprite SPunto, SCursor(TCursor), SLogo(TLogo);

    srand(time(NULL));
    std::vector<std::vector<int>> Tablero(TableroTamano, std::vector<int>(TableroTamano));

    // Creación del hilo inicial para asegurar los datos antes del loop principal
    pthread_t HiloInicial;
    DatosTablero* ArgumentosInicio = new DatosTablero{&Tablero, &MutexTablero};
    pthread_create(&HiloInicial, NULL, HiloGenerarTablero, (void*)ArgumentosInicio);
    pthread_join(HiloInicial, NULL); 

    // Inicialización de variables de estado, control de flujo y estadísticas del jugador
    EstadoPantalla EstadoActual = Menu;
    ModoJuego ModoSeleccionado = Lento;
    int SeleccionMenu = 0; 
    Cursor CursorJuego;
    std::vector<Punto> CadenaSeleccionada;
    
    int Puntuacion = 0;
    int Movimientos = 30;
    const int MetaPuntuacion = 100;

    // Buffer de memoria para clonar el tablero y procesar gráficos sin retener el mutex principal
    int TableroCopia[TableroTamano][TableroTamano];

    // Bucle principal de ejecución de SFML enfocado en la tasa de refresco
    while (Ventana.isOpen()) {
        sf::Event Evento;
        while (Ventana.pollEvent(Evento)) {
            if (Evento.type == sf::Event::Closed) Ventana.close();

            if (Evento.type == sf::Event::KeyPressed) {

                // Manejo de teclas en la pantalla del menú inicial
                if (EstadoActual == Menu) {
                    if (Evento.key.code == sf::Keyboard::Enter || Evento.key.code == sf::Keyboard::Space) {
                        EstadoActual = Modalidad;
                    }
                } 
                // Manejo de teclas ara la selección de dificultad y límites de juego
                else if (EstadoActual == Modalidad) {
                    if (Evento.key.code == sf::Keyboard::W || Evento.key.code == sf::Keyboard::Up) {
                        SeleccionMenu = 0;
                    }
                    else if (Evento.key.code == sf::Keyboard::S || Evento.key.code == sf::Keyboard::Down) {
                        SeleccionMenu = 1;
                    }
                    else if (Evento.key.code == sf::Keyboard::Enter) {
                        if (SeleccionMenu == 0) {
                            ModoSeleccionado = Lento;
                            Movimientos = 30;
                        } else {
                            ModoSeleccionado = Rapido;
                            Movimientos = 15;
                        }
                        EstadoActual = TableroJuego;
                    }
                }
                // Comandos dentro del tablero
                else if (EstadoActual == TableroJuego) {
                    if (Evento.key.code == sf::Keyboard::W || Evento.key.code == sf::Keyboard::Up) { if (CursorJuego.Y > 0) CursorJuego.Y--; }
                    else if (Evento.key.code == sf::Keyboard::S || Evento.key.code == sf::Keyboard::Down) { if (CursorJuego.Y < TableroTamano - 1) CursorJuego.Y++; }
                    else if (Evento.key.code == sf::Keyboard::A || Evento.key.code == sf::Keyboard::Left) { if (CursorJuego.X > 0) CursorJuego.X--; }
                    else if (Evento.key.code == sf::Keyboard::D || Evento.key.code == sf::Keyboard::Right) { if (CursorJuego.X < TableroTamano - 1) CursorJuego.X++; }
                    
                    // Lógica para agregar o remover elementos de la cadena de dots
                    else if (Evento.key.code == sf::Keyboard::Space) {
                        if (!CadenaSeleccionada.empty() && CadenaSeleccionada.back().X == CursorJuego.X && CadenaSeleccionada.back().Y == CursorJuego.Y) {
                            CadenaSeleccionada.pop_back();
                        }
                        else if (!EsPuntoEnCadena(CursorJuego.X, CursorJuego.Y, CadenaSeleccionada)) {
                            if (CadenaSeleccionada.empty()) {
                                CadenaSeleccionada.push_back({CursorJuego.X, CursorJuego.Y});
                            } else {
                                Punto Ultimo = CadenaSeleccionada.back();
                                
                                // Bloqueo momentáneo y lectura de la matriz para validar colores al lado
                                pthread_mutex_lock(&MutexTablero);
                                int ColorUltimo = Tablero[Ultimo.Y][Ultimo.X];
                                int ColorActual = Tablero[CursorJuego.Y][CursorJuego.X];
                                pthread_mutex_unlock(&MutexTablero);

                                if (ColorUltimo == ColorActual) {
                                    if ((std::abs(Ultimo.X - CursorJuego.X) + std::abs(Ultimo.Y - CursorJuego.Y)) == 1) {
                                        CadenaSeleccionada.push_back({CursorJuego.X, CursorJuego.Y});
                                    }
                                }
                            }
                        }
                    }
                    // Ejecución en segundo plano de la jugada confirmada
                    else if (Evento.key.code == sf::Keyboard::Enter) {
                        if (CadenaSeleccionada.size() >= 2) {
                            pthread_t HiloJugada;
                            DatosJugada* ArgumentosJugada = new DatosJugada{
                                &Tablero, 
                                CadenaSeleccionada, 
                                &Puntuacion, 
                                &Movimientos, 
                                &EstadoActual, 
                                &MutexTablero
                            };

                            pthread_create(&HiloJugada, NULL, HiloProcesarJugada, (void*)ArgumentosJugada);
                            
                            // Desacoplamiento del hilo para evitar detener el renderizado
                            pthread_detach(HiloJugada); 

                            CadenaSeleccionada.clear();
                        }
                    }
                    else if (Evento.key.code == sf::Keyboard::Q) {
                        CadenaSeleccionada.clear();
                        EstadoActual = Menu;
                    }
                }
                // Control y desvío de flujo cuando la partida ha terminado (Victoria o Derrota)
                else if (EstadoActual == PartidaTerminada) {
                    if (Evento.key.code == sf::Keyboard::Enter || Evento.key.code == sf::Keyboard::Space) {
                        Puntuacion = 0;
                        CadenaSeleccionada.clear();
                        
                        // Regenera un nuevo tablero limpio
                        pthread_t HiloReset;
                        DatosTablero* ArgumentosReset = new DatosTablero{&Tablero, &MutexTablero};
                        pthread_create(&HiloReset, NULL, HiloGenerarTablero, (void*)ArgumentosReset);
                        pthread_detach(HiloReset);

                        EstadoActual = Menu;
                    }
                }
            }
        }

        // Renderizado del menú principal
        if (EstadoActual == Menu) {
            Ventana.clear(sf::Color(255, 255, 255));
            SLogo.setPosition((800.f - SLogo.getGlobalBounds().width) / 2.f, 100.f);
            Ventana.draw(SLogo);

            sf::Text TextoInfo("Presiona ENTER para continuar", FuenteJuego, 24);
            TextoInfo.setFillColor(sf::Color::Black);
            TextoInfo.setPosition((800.f - TextoInfo.getLocalBounds().width) / 2.f, 380.f);
            Ventana.draw(TextoInfo);
        } 
        // Renderizado del selector de modalidades de juego
        else if (EstadoActual == Modalidad) {
            Ventana.clear(sf::Color(255, 255, 255));

            sf::Text TituloMod("SELECCIONA EL MODO DE JUEGO", FuenteJuego, 32);
            TituloMod.setFillColor(sf::Color::Blue);
            TituloMod.setPosition((800.f - TituloMod.getLocalBounds().width) / 2.f, 120.f);
            Ventana.draw(TituloMod);

            std::string StrModo1 = (SeleccionMenu == 0) ? "> Modo Lento (30 Movimientos) <" : "Modo Lento (30 Movimientos)";
            sf::Text Modo1(StrModo1, FuenteJuego, 22);
            Modo1.setFillColor((SeleccionMenu == 0) ? sf::Color::Yellow : sf::Color::Black);
            Modo1.setPosition((800.f - Modo1.getLocalBounds().width) / 2.f, 260.f);
            Ventana.draw(Modo1);

            std::string StrModo2 = (SeleccionMenu == 1) ? "> Modo Rapido (15 Movimientos) <" : "Modo Rapido (15 Movimientos)";
            sf::Text Modo2(StrModo2, FuenteJuego, 22);
            Modo2.setFillColor((SeleccionMenu == 1) ? sf::Color::Yellow : sf::Color::Black);
            Modo2.setPosition((800.f - Modo2.getLocalBounds().width) / 2.f, 340.f);
            Ventana.draw(Modo2);

            sf::Text TextoPie("Usa las flechas y presiona ENTER", FuenteJuego, 16);
            TextoPie.setFillColor(sf::Color(148, 163, 184));
            TextoPie.setPosition((800.f - TextoPie.getLocalBounds().width) / 2.f, 480.f);
            Ventana.draw(TextoPie);
        }
        // Renderizado del tablero. Se clona rápidamente la matriz para evitar bloqueos internos
        else if (EstadoActual == TableroJuego) {
            Ventana.clear(sf::Color(255, 255, 255));

            sf::Text TxtPuntuacion("Puntos: " + std::to_string(Puntuacion), FuenteJuego, 24);
            TxtPuntuacion.setFillColor(sf::Color::Blue);
            TxtPuntuacion.setPosition(120, 40);
            Ventana.draw(TxtPuntuacion);

            sf::Text TxtMovimientos("Movimientos: " + std::to_string(Movimientos), FuenteJuego, 24);
            TxtMovimientos.setFillColor(sf::Color::Red);
            TxtMovimientos.setPosition(500, 40);
            Ventana.draw(TxtMovimientos);

            // Copiar la matriz
            pthread_mutex_lock(&MutexTablero);
            for (int I = 0; I < TableroTamano; ++I) {
                for (int J = 0; J < TableroTamano; ++J) {
                    TableroCopia[I][J] = Tablero[I][J];
                }
            }
            pthread_mutex_unlock(&MutexTablero); 

            // Construcción y proyección de los sprites leyendo la estructura clonada
            for (int I = 0; I < TableroTamano; ++I) {
                for (int J = 0; J < TableroTamano; ++J) {
                    float PosX = DesplazamientoX + (J * CeldaTamano);
                    float PosY = DesplazamientoY + (I * CeldaTamano);

                    bool Seleccionado = EsPuntoEnCadena(J, I, CadenaSeleccionada);

                    switch (TableroCopia[I][J]) {
                        case 1: SPunto.setTexture(TAzul); break;
                        case 2: SPunto.setTexture(TRojo); break;
                        case 3: SPunto.setTexture(TVerde); break;
                        case 4: SPunto.setTexture(TAmarillo); break;
                    }

                    SPunto.setPosition(PosX, PosY);
                    SPunto.setScale(48.f / SPunto.getLocalBounds().width, 48.f / SPunto.getLocalBounds().height);
                    
                    if (Seleccionado) {
                        SPunto.setColor(sf::Color(130, 130, 130, 255)); 
                    } else {
                        SPunto.setColor(sf::Color(255, 255, 255, 255));
                    }
                    Ventana.draw(SPunto);

                    if (I == CursorJuego.Y && J == CursorJuego.X) {
                        SCursor.setPosition(PosX - 4, PosY - 4);
                        SCursor.setScale(56.f / SCursor.getLocalBounds().width, 56.f / SCursor.getLocalBounds().height);
                        Ventana.draw(SCursor);
                    }
                }
            }
        }
        // Estado final de la partida (Victoria o derrota)
        else if (EstadoActual == PartidaTerminada) {
            Ventana.clear(sf::Color(255, 255, 255));

            bool Gano = (Puntuacion >= MetaPuntuacion);
            
            sf::Text TextoEstado(Gano ? "VICTORIA!" : "FIN DE LA PARTIDA", FuenteJuego, 40);
            TextoEstado.setFillColor(Gano ? sf::Color::Green : sf::Color::Red);
            TextoEstado.setPosition((800.f - TextoEstado.getLocalBounds().width) / 2.f, 150.f);
            Ventana.draw(TextoEstado);

            sf::Text TextoPuntuacionFinal("Puntuacion Final: " + std::to_string(Puntuacion), FuenteJuego, 28);
            TextoPuntuacionFinal.setFillColor(sf::Color::Black);
            TextoPuntuacionFinal.setPosition((800.f - TextoPuntuacionFinal.getLocalBounds().width) / 2.f, 260.f);
            Ventana.draw(TextoPuntuacionFinal);

            sf::Text TextoReiniciar("Presiona ENTER para volver al menu", FuenteJuego, 18);
            TextoReiniciar.setFillColor(sf::Color::Yellow);
            TextoReiniciar.setPosition((800.f - TextoReiniciar.getLocalBounds().width) / 2.f, 420.f);
            Ventana.draw(TextoReiniciar);
        }

        Ventana.display();
    }

    // Liberación de los recursos
    pthread_mutex_destroy(&MutexTablero);
    return 0;
}