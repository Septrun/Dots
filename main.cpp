#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <cmath>

const int TableroTamano = 8;
const int CeldaTamano = 60;   
const int DesplazamientoX = 160;   
const int DesplazamientoY = 100;  

enum EstadoPantalla { Menu, Modalidad, TableroJuego, PartidaTerminada };
enum ModoJuego { Lento, Rapido };

struct Cursor { int X = 0; int Y = 0; };
struct Punto { int X; int Y; };

bool EsPuntoEnCadena(int X, int Y, const std::vector<Punto>& Cadena) {
    for (const auto& Pt : Cadena) {
        if (Pt.X == X && Pt.Y == Y) return true;
    }
    return false;
}

int main() {
    sf::RenderWindow Ventana(sf::VideoMode(800, 600), "Dots Game - Pocket Edition");
    Ventana.setFramerateLimit(60);

    // Se carga la fuente personalizada (.ttf)

    sf::Font FuenteJuego;
    if (!FuenteJuego.loadFromFile("assets/Baxoe.ttf")) {
        std::cerr << "Error. No se pudo cargar la fuente assets/Baxoe.ttf" << std::endl;
        return -1;
    }

    // Se cargan las texturas (Hechas en Photopea y Chatgpt)

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


    // Lógica del programa e inicialización del tablero

    srand(time(NULL));
    std::vector<std::vector<int>> Tablero(TableroTamano, std::vector<int>(TableroTamano));
    for (int I = 0; I < TableroTamano; ++I)
        for (int J = 0; J < TableroTamano; ++J)
            Tablero[I][J] = (rand() % 4) + 1;

    EstadoPantalla EstadoActual = Menu;
    ModoJuego ModoSeleccionado = Lento;
    int SeleccionMenu = 0; // Modos de juego: Modo lento (0) y modo rápido (1)
    Cursor CursorJuego;
    std::vector<Punto> CadenaSeleccionada;
    
    int Puntuacion = 0;
    int Movimientos = 30;
    const int MetaPuntuacion = 100;

    // While principal
    
    while (Ventana.isOpen()) {
        sf::Event Evento;
        while (Ventana.pollEvent(Evento)) {
            if (Evento.type == sf::Event::Closed) Ventana.close();

            if (Evento.type == sf::Event::KeyPressed) {

                // Pantalla 1: Menu
                if (EstadoActual == Menu) {
                    if (Evento.key.code == sf::Keyboard::Enter || Evento.key.code == sf::Keyboard::Space) {
                        EstadoActual = Modalidad;
                    }
                } 
                // Pantalla 2: Selección de modo de juego
                else if (EstadoActual == Modalidad) {
                    if (Evento.key.code == sf::Keyboard::W || Evento.key.code == sf::Keyboard::Up) {
                        SeleccionMenu = 0; // Sube al modo lento
                    }
                    else if (Evento.key.code == sf::Keyboard::S || Evento.key.code == sf::Keyboard::Down) {
                        SeleccionMenu = 1; // Baja al modo rápido
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
                // Pantalla 3: Tablero del juego
                else if (EstadoActual == TableroJuego) {
                    if (Evento.key.code == sf::Keyboard::W || Evento.key.code == sf::Keyboard::Up) { if (CursorJuego.Y > 0) CursorJuego.Y--; }
                    else if (Evento.key.code == sf::Keyboard::S || Evento.key.code == sf::Keyboard::Down) { if (CursorJuego.Y < TableroTamano - 1) CursorJuego.Y++; }
                    else if (Evento.key.code == sf::Keyboard::A || Evento.key.code == sf::Keyboard::Left) { if (CursorJuego.X > 0) CursorJuego.X--; }
                    else if (Evento.key.code == sf::Keyboard::D || Evento.key.code == sf::Keyboard::Right) { if (CursorJuego.X < TableroTamano - 1) CursorJuego.X++; }
                    
                    else if (Evento.key.code == sf::Keyboard::Space) {

                        // Sistema para seleccionar los dots
                        if (!CadenaSeleccionada.empty() && CadenaSeleccionada.back().X == CursorJuego.X && CadenaSeleccionada.back().Y == CursorJuego.Y) {

                            // Permite eliminar la cadena al presionar de nuevo sobre el dot
                            CadenaSeleccionada.pop_back();
                        }
                        else if (!EsPuntoEnCadena(CursorJuego.X, CursorJuego.Y, CadenaSeleccionada)) {
                            if (CadenaSeleccionada.empty()) {
                                CadenaSeleccionada.push_back({CursorJuego.X, CursorJuego.Y});
                            } else {
                                Punto Ultimo = CadenaSeleccionada.back();
                                if (Tablero[Ultimo.Y][Ultimo.X] == Tablero[CursorJuego.Y][CursorJuego.X]) {
                                    if ((std::abs(Ultimo.X - CursorJuego.X) + std::abs(Ultimo.Y - CursorJuego.Y)) == 1) {
                                        CadenaSeleccionada.push_back({CursorJuego.X, CursorJuego.Y});
                                    }
                                }
                            }
                        }
                    }
                    else if (Evento.key.code == sf::Keyboard::Enter) {
                        if (CadenaSeleccionada.size() >= 2) {
                            Puntuacion += CadenaSeleccionada.size() * 5;
                            Movimientos--;
                            
                            for (const auto& Pt : CadenaSeleccionada) {
                                Tablero[Pt.Y][Pt.X] = (rand() % 4) + 1;
                            }
                            CadenaSeleccionada.clear();

                            if (Movimientos <= 0) {
                                EstadoActual = PartidaTerminada;
                            }
                        }
                    }
                    else if (Evento.key.code == sf::Keyboard::Q) {
                        CadenaSeleccionada.clear();
                        EstadoActual = Menu;
                    }
                }
                // Pantalla 4: Partida terminada
                else if (EstadoActual == PartidaTerminada) {
                    if (Evento.key.code == sf::Keyboard::Enter || Evento.key.code == sf::Keyboard::Space) {
                        Puntuacion = 0;
                        CadenaSeleccionada.clear();
                        EstadoActual = Menu;
                    }
                }
            }
        }

        // Gráficos del juego

        if (EstadoActual == Menu) {
            Ventana.clear(sf::Color(15, 23, 42));

            SLogo.setPosition((800.f - SLogo.getGlobalBounds().width) / 2.f, 100.f);
            Ventana.draw(SLogo);

            sf::Text TextoInfo("Presiona ENTER para continuar", FuenteJuego, 24);
            TextoInfo.setFillColor(sf::Color::White);
            TextoInfo.setPosition((800.f - TextoInfo.getLocalBounds().width) / 2.f, 380.f);
            Ventana.draw(TextoInfo);
        } 
        else if (EstadoActual == Modalidad) {
            Ventana.clear(sf::Color(15, 23, 42));

            sf::Text TituloMod("SELECCIONA EL MODO DE JUEGO", FuenteJuego, 32);
            TituloMod.setFillColor(sf::Color::Cyan);
            TituloMod.setPosition((800.f - TituloMod.getLocalBounds().width) / 2.f, 120.f);
            Ventana.draw(TituloMod);

            // Resaltar Modo Lento si SeleccionMenu es 0
            std::string StrModo1 = (SeleccionMenu == 0) ? "> Modo Lento (30 Movimientos) <" : "Modo Lento (30 Movimientos)";
            sf::Text Modo1(StrModo1, FuenteJuego, 22);
            Modo1.setFillColor((SeleccionMenu == 0) ? sf::Color::Yellow : sf::Color::White);
            Modo1.setPosition((800.f - Modo1.getLocalBounds().width) / 2.f, 260.f);
            Ventana.draw(Modo1);

            // Resaltar Modo Rápido si SeleccionMenu es 1
            std::string StrModo2 = (SeleccionMenu == 1) ? "> Modo Rapido (15 Movimientos) <" : "Modo Rapido (15 Movimientos)";
            sf::Text Modo2(StrModo2, FuenteJuego, 22);
            Modo2.setFillColor((SeleccionMenu == 1) ? sf::Color::Yellow : sf::Color::White);
            Modo2.setPosition((800.f - Modo2.getLocalBounds().width) / 2.f, 340.f);
            Ventana.draw(Modo2);

            sf::Text TextoPie("Usa las flechas y presiona ENTER", FuenteJuego, 16);
            TextoPie.setFillColor(sf::Color(148, 163, 184));
            TextoPie.setPosition((800.f - TextoPie.getLocalBounds().width) / 2.f, 480.f);
            Ventana.draw(TextoPie);
        }
        else if (EstadoActual == TableroJuego) {
            Ventana.clear(sf::Color(30, 33, 45));

            sf::Text TxtPuntuacion("Puntos: " + std::to_string(Puntuacion), FuenteJuego, 24);
            TxtPuntuacion.setFillColor(sf::Color::Cyan);
            TxtPuntuacion.setPosition(120, 40);
            Ventana.draw(TxtPuntuacion);

            sf::Text TxtMovimientos("Movimientos: " + std::to_string(Movimientos), FuenteJuego, 24);
            TxtMovimientos.setFillColor(sf::Color::Red);
            TxtMovimientos.setPosition(500, 40);
            Ventana.draw(TxtMovimientos);

            // Renderizado de la cuadrícula
            for (int I = 0; I < TableroTamano; ++I) {
                for (int J = 0; J < TableroTamano; ++J) {
                    float PosX = DesplazamientoX + (J * CeldaTamano);
                    float PosY = DesplazamientoY + (I * CeldaTamano);

                    bool Seleccionado = EsPuntoEnCadena(J, I, CadenaSeleccionada);

                    switch (Tablero[I][J]) {
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
        else if (EstadoActual == PartidaTerminada) {
            Ventana.clear(sf::Color(24, 24, 27));

            bool Gano = (Puntuacion >= MetaPuntuacion);
            
            sf::Text TextoEstado(Gano ? "VICTORIA!" : "FIN DE LA PARTIDA", FuenteJuego, 40);
            TextoEstado.setFillColor(Gano ? sf::Color::Green : sf::Color::Red);
            TextoEstado.setPosition((800.f - TextoEstado.getLocalBounds().width) / 2.f, 150.f);
            Ventana.draw(TextoEstado);

            sf::Text TextoPuntuacionFinal("Puntuacion Final: " + std::to_string(Puntuacion), FuenteJuego, 28);
            TextoPuntuacionFinal.setFillColor(sf::Color::White);
            TextoPuntuacionFinal.setPosition((800.f - TextoPuntuacionFinal.getLocalBounds().width) / 2.f, 260.f);
            Ventana.draw(TextoPuntuacionFinal);

            sf::Text TextoReiniciar("Presiona ENTER para volver al menu", FuenteJuego, 18);
            TextoReiniciar.setFillColor(sf::Color::Yellow);
            TextoReiniciar.setPosition((800.f - TextoReiniciar.getLocalBounds().width) / 2.f, 420.f);
            Ventana.draw(TextoReiniciar);
        }

        Ventana.display();
    }

    return 0;
}