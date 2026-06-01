#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include "JuegoTipos.h"
#include "TableroJuego.h"

int main() {
    // Configuracion inicial de la ventana y del juego
    sf::RenderWindow Ventana(sf::VideoMode(800, 600), "Dots Game - Pocket Edition");
    Ventana.setFramerateLimit(60);

    TableroJuegoManager JuegoManager;

    sf::Font FuenteJuego;
    if (!FuenteJuego.loadFromFile("assets/Baxoe.ttf")) {
        std::cerr << "Hubo un error. No se pudo cargar la fuente assets/Baxoe.ttf" << std::endl;
        return -1;
    }

    // Carga de imagenes que se usan para los puntos, cursor, logo y elementos especiales
    sf::Texture TCursor, TAzul, TRojo, TVerde, TAmarillo, TMorado, TNaranja, TLogo, TArcoiris, TNegro;
    if (!TCursor.loadFromFile("assets/Cursor.png") ||
        !TAzul.loadFromFile("assets/Azul.png") ||
        !TRojo.loadFromFile("assets/Rojo.png") ||
        !TVerde.loadFromFile("assets/Verde.png") ||
        !TAmarillo.loadFromFile("assets/Amarillo.png") ||
        !TMorado.loadFromFile("assets/Morado.png") ||    
        !TNaranja.loadFromFile("assets/Naranja.png") ||  
        !TLogo.loadFromFile("assets/Logo.png") ||
        !TArcoiris.loadFromFile("assets/Arcoiris.png") ||
        !TNegro.loadFromFile("assets/Negro.png")) {
        std::cerr << "Hubo un error. No se pudieron cargar las imagenes desde la carpeta assets/" << std::endl;
        return -1;
    }

    sf::Sprite SPunto, SCursor(TCursor), SLogo(TLogo);

    srand(time(NULL));

    // Variables principales para controlar pantallas, menus y modalidad del juego
    EstadoPantalla EstadoActual = Menu;
    ModoJuego ModoSeleccionado = Lento;
    int SeleccionMenuInicio = 0;  
    int SeleccionDificultad = 0;  
    int SeleccionPausa = 0;       
    int PaginaInstrucciones = 0;

    // Variables que guardan el cursor y la cadena de puntos seleccionados
    Cursor CursorJuego;
    std::vector<Punto> CadenaSeleccionada;
    
    // Valores base de la partida
    int Puntuacion = 0;
    int Movimientos = 30;
    int CantidadColoresActiva = 4; 
    
    int MetaPuntuacion = 600;
    int MultiplicadorEntero = 10; 
    
    // Variables para manejar bonos y multiplicadores especiales
    bool EstalloArcoirisSiguiente = false;
    bool TurnoBonoActivo = false;
    int MultiplicadorRespaldado = 10;

    sf::Clock RelojCarga;
    std::string NombreModoActual = "";
    sf::Color ColorModoActual = sf::Color::Black;

    int TableroCopia[TableroTamano][TableroTamano];

    // Ciclo principal del programa
    while (Ventana.isOpen()) {
        sf::Event Evento;

        // Se revisan todos los eventos del teclado y la ventana
        while (Ventana.pollEvent(Evento)) {
            if (Evento.type == sf::Event::Closed) Ventana.close();

            if (Evento.type == sf::Event::KeyPressed) {
                
                // Logica del menu principal
                if (EstadoActual == Menu) {
                    if (Evento.key.code == sf::Keyboard::W || Evento.key.code == sf::Keyboard::Up) {
                        SeleccionMenuInicio = 0;
                    }
                    else if (Evento.key.code == sf::Keyboard::S || Evento.key.code == sf::Keyboard::Down) {
                        SeleccionMenuInicio = 1;
                    }
                    else if (Evento.key.code == sf::Keyboard::Enter || Evento.key.code == sf::Keyboard::Space) {
                        if (SeleccionMenuInicio == 0) {
                            EstadoActual = Modalidad;
                        } else {
                            PaginaInstrucciones = 0;
                            EstadoActual = Instrucciones; 
                        }
                    }
                }

                // Logica para navegar entre paginas de instrucciones
                else if (EstadoActual == Instrucciones) {
                    if (Evento.key.code == sf::Keyboard::D || Evento.key.code == sf::Keyboard::Right) {
                        if (PaginaInstrucciones < 3) PaginaInstrucciones++;
                    }
                    else if (Evento.key.code == sf::Keyboard::A || Evento.key.code == sf::Keyboard::Left) {
                        if (PaginaInstrucciones > 0) PaginaInstrucciones--;
                    }
                    else if (Evento.key.code == sf::Keyboard::Q || Evento.key.code == sf::Keyboard::Escape) {
                        EstadoActual = Menu;
                    }
                }

                // Logica para seleccionar modo de juego
                else if (EstadoActual == Modalidad) {
                    if (Evento.key.code == sf::Keyboard::W || Evento.key.code == sf::Keyboard::Up) {
                        if (SeleccionDificultad > 0) SeleccionDificultad--;
                    }
                    else if (Evento.key.code == sf::Keyboard::S || Evento.key.code == sf::Keyboard::Down) {
                        if (SeleccionDificultad < 2) SeleccionDificultad++;
                    }
                    else if (Evento.key.code == sf::Keyboard::Q || Evento.key.code == sf::Keyboard::Escape) {
                        EstadoActual = Menu; 
                    }
                    else if (Evento.key.code == sf::Keyboard::Enter) {
                        MultiplicadorEntero = 10; 
                        EstalloArcoirisSiguiente = false;
                        TurnoBonoActivo = false;
                        Puntuacion = 0; 

                        if (SeleccionDificultad == 0) {
                            ModoSeleccionado = Lento; 
                            Movimientos = 30; 
                            CantidadColoresActiva = 4; 
                            MetaPuntuacion = 600; 
                            NombreModoActual = "MODO LENTO"; 
                            ColorModoActual = sf::Color::Blue;
                            JuegoManager.InicializarTableroAsync(4);
                        } 
                        else if (SeleccionDificultad == 1) {
                            ModoSeleccionado = Rapido; 
                            Movimientos = 15; 
                            CantidadColoresActiva = 6; 
                            MetaPuntuacion = 400; 
                            NombreModoActual = "MODO RAPIDO"; 
                            ColorModoActual = sf::Color(234, 179, 8);
                            JuegoManager.InicializarTableroAsync(106);
                        } 
                        else {
                            ModoSeleccionado = Caos; 
                            Movimientos = 20; 
                            CantidadColoresActiva = 6; 
                            MetaPuntuacion = 400; 
                            NombreModoActual = "MODO CAOS"; 
                            ColorModoActual = sf::Color::Red;
                            JuegoManager.InicializarTableroAsync(206);
                        }
                        
                        RelojCarga.restart();
                        EstadoActual = PantallaCarga;
                    }
                }

                // Logica principal cuando el jugador esta en el tablero
                else if (EstadoActual == TableroJuego) {
                    if (Evento.key.code == sf::Keyboard::W || Evento.key.code == sf::Keyboard::Up) { 
                        if (CursorJuego.Y > 0) CursorJuego.Y--; 
                    }
                    else if (Evento.key.code == sf::Keyboard::S || Evento.key.code == sf::Keyboard::Down) { 
                        if (CursorJuego.Y < TableroTamano - 1) CursorJuego.Y++; 
                    }
                    else if (Evento.key.code == sf::Keyboard::A || Evento.key.code == sf::Keyboard::Left) { 
                        if (CursorJuego.X > 0) CursorJuego.X--; 
                    }
                    else if (Evento.key.code == sf::Keyboard::D || Evento.key.code == sf::Keyboard::Right) { 
                        if (CursorJuego.X < TableroTamano - 1) CursorJuego.X++; 
                    }
                    
                    // Seleccion de puntos para formar una cadena
                    else if (Evento.key.code == sf::Keyboard::Space) {
                        if (!CadenaSeleccionada.empty() && CadenaSeleccionada.back().X == CursorJuego.X && CadenaSeleccionada.back().Y == CursorJuego.Y) {
                            CadenaSeleccionada.pop_back();
                        }
                        else {
                            if (CadenaSeleccionada.empty()) {
                                CadenaSeleccionada.push_back({CursorJuego.X, CursorJuego.Y});
                            } else {
                                Punto Ultimo = CadenaSeleccionada.back();
                                Punto Primero = CadenaSeleccionada.front();

                                if (CursorJuego.X == Primero.X && CursorJuego.Y == Primero.Y && CadenaSeleccionada.size() >= 3) {
                                    if (JuegoManager.ValidarConexion(Ultimo, CursorJuego)) {
                                        CadenaSeleccionada.push_back({CursorJuego.X, CursorJuego.Y});
                                    }
                                }
                                else if (!EsPuntoEnCadena(CursorJuego.X, CursorJuego.Y, CadenaSeleccionada)) {
                                    if (JuegoManager.ValidarConexion(Ultimo, CursorJuego)) {
                                        CadenaSeleccionada.push_back({CursorJuego.X, CursorJuego.Y});
                                    }
                                }
                            }
                        }
                    }

                    // Confirmacion de la jugada y calculo de bonos
                    else if (Evento.key.code == sf::Keyboard::Enter) {
                        if (CadenaSeleccionada.size() >= 2) {
                            
                            bool ContieneArcoirisLocal = false;
                            JuegoManager.ClonarTablero(TableroCopia);
                            for (const auto& Pt : CadenaSeleccionada) {
                                if (TableroCopia[Pt.Y][Pt.X] == 7) {
                                    ContieneArcoirisLocal = true;
                                    break;
                                }
                            }

                            JuegoManager.ProcesarJugadaAsync(CadenaSeleccionada, Puntuacion, Movimientos, EstadoActual, CantidadColoresActiva, ModoSeleccionado, MultiplicadorEntero);
                            
                            if (ModoSeleccionado == Rapido || ModoSeleccionado == Caos) {
                                if (TurnoBonoActivo) {
                                    MultiplicadorEntero = MultiplicadorRespaldado;
                                    TurnoBonoActivo = false;
                                    
                                    if (!ContieneArcoirisLocal) {
                                        if (CadenaSeleccionada.size() >= 4) {
                                            MultiplicadorEntero += 2;
                                        } else {
                                            if (MultiplicadorEntero > 10) MultiplicadorEntero -= 2;
                                            if (MultiplicadorEntero < 10) MultiplicadorEntero = 10;
                                        }
                                    }
                                } 
                                else {
                                    if (CadenaSeleccionada.size() >= 4) {
                                        MultiplicadorEntero += 2;
                                    } else {
                                        if (MultiplicadorEntero > 10) MultiplicadorEntero -= 2;
                                        if (MultiplicadorEntero < 10) MultiplicadorEntero = 10;
                                    }
                                }

                                if (ContieneArcoirisLocal) {
                                    MultiplicadorRespaldado = MultiplicadorEntero;
                                    MultiplicadorEntero = 30; 
                                    TurnoBonoActivo = true;
                                }
                            }

                            CadenaSeleccionada.clear();
                        }
                    }

                    // Opciones rapidas dentro de la partida
                    else if (Evento.key.code == sf::Keyboard::Escape) {
                        CadenaSeleccionada.clear();
                    }
                    else if (Evento.key.code == sf::Keyboard::Q) {
                        SeleccionPausa = 0;
                        EstadoActual = Pausa;
                    }
                }

                // Logica del menu de pausa
                else if (EstadoActual == Pausa) {
                    if (Evento.key.code == sf::Keyboard::W || Evento.key.code == sf::Keyboard::Up) {
                        SeleccionPausa = 0;
                    }
                    else if (Evento.key.code == sf::Keyboard::S || Evento.key.code == sf::Keyboard::Down) {
                        SeleccionPausa = 1;
                    }
                    else if (Evento.key.code == sf::Keyboard::Enter) {
                        if (SeleccionPausa == 0) {
                            EstadoActual = TableroJuego; 
                        } else {
                            CadenaSeleccionada.clear();
                            EstadoActual = Menu; 
                        }
                    }
                    else if (Evento.key.code == sf::Keyboard::Q) {
                        EstadoActual = TableroJuego; 
                    }
                }

                // Al terminar la partida se puede volver al menu
                else if (EstadoActual == PartidaTerminada) {
                    if (Evento.key.code == sf::Keyboard::Enter || Evento.key.code == sf::Keyboard::Space) {
                        CadenaSeleccionada.clear();
                        EstadoActual = Menu;
                    }
                }
            }
        }

        // Cambio automatico despues de la pantalla de carga
        if (EstadoActual == PantallaCarga) {
            if (RelojCarga.getElapsedTime().asSeconds() >= 3.0f) {
                EstadoActual = TableroJuego;
            }
        }

        // Revisa si el jugador gano o perdio
        if (EstadoActual == TableroJuego) {
            if (Puntuacion >= MetaPuntuacion || Movimientos <= 0) {
                EstadoActual = PartidaTerminada;
            }
        }

        // A partir de aqui se dibuja la pantalla correspondiente
        Ventana.clear(sf::Color(255, 255, 255));

        // Dibujo del menu principal
        if (EstadoActual == Menu) {
            SLogo.setPosition((800.f - SLogo.getGlobalBounds().width) / 2.f, 80.f);
            Ventana.draw(SLogo);

            std::string StrOpc1 = (SeleccionMenuInicio == 0) ? "> EMPEZAR JUEGO <" : "EMPEZAR JUEGO";
            sf::Text Opc1(StrOpc1, FuenteJuego, 24);
            Opc1.setFillColor((SeleccionMenuInicio == 0) ? sf::Color::Blue : sf::Color::Black);
            Opc1.setPosition((800.f - Opc1.getLocalBounds().width) / 2.f, 340.f);
            Ventana.draw(Opc1);

            std::string StrOpc2 = (SeleccionMenuInicio == 1) ? "> INSTRUCCIONES <" : "INSTRUCCIONES";
            sf::Text Opc2(StrOpc2, FuenteJuego, 24);
            Opc2.setFillColor((SeleccionMenuInicio == 1) ? sf::Color::Blue : sf::Color::Black);
            Opc2.setPosition((800.f - Opc2.getLocalBounds().width) / 2.f, 410.f);
            Ventana.draw(Opc2);

            sf::Text AyudaMenu("Usa las flechas para moverte y ENTER para seleccionar", FuenteJuego, 14);
            AyudaMenu.setFillColor(sf::Color(120, 120, 120));
            AyudaMenu.setPosition((800.f - AyudaMenu.getLocalBounds().width) / 2.f, 520.f);
            Ventana.draw(AyudaMenu);
        } 

        // Dibujo de instrucciones
        else if (EstadoActual == Instrucciones) {
            sf::Text TituloIns("", FuenteJuego, 32);
            std::string TextoReglas = "";

            if (PaginaInstrucciones == 0) {
                TituloIns.setString("1. REGLAS GENERALES");
                TituloIns.setFillColor(sf::Color::Blue);
                TextoReglas = 
                    "- Usa las teclas W, A, S, D (o Flechas) para moverte por el tablero.\n\n"
                    "- Presiona ESPACIO sobre un punto para iniciar o agregar celdas.\n\n"
                    "- Conecta puntos ADYACENTES en cruz. La diagonales NO estan permitidas.\n\n"
                    "- Presiona ENTER para validar la jugada y ESCAPE para cancelarla.\n\n"
                    "- Presiona la tecla Q en cualquier momento para pausar la partida.\n\n"
                    "- Formar un CUADRADO 2X2 otorgara un bono masivo\n"
                    "  de 8 puntos base por cada dot en lugar de los 5 habituales!";
            } 
            else if (PaginaInstrucciones == 1) {
                TituloIns.setString("2. ESPECIFICACIONES: Modo Lento");
                TituloIns.setFillColor(sf::Color(59, 130, 246));
                TextoReglas = 
                    "- Objetivo del Nivel: Alcanzar la meta de 600 puntos antes de agotar turnos.\n\n"
                    "- Restricciones: Cuentas con un limite de 30 movimientos.\n\n"
                    "- Dificultad: Baja. El tablero se genera utilizando unicamente 4 colores.\n\n";
            } 
            else if (PaginaInstrucciones == 2) {
                TituloIns.setString("3. ESPECIFICACIONES: Modo Rapido");
                TituloIns.setFillColor(sf::Color(234, 179, 8));
                TextoReglas = 
                    "- Objetivo del Nivel: Alcanzar la meta de 400 puntos.\n\n"
                    "- Restricciones: Cuentas con un limite de 15 movimientos.\n\n"
                    "- Dificultad: Media. El tablero se juega con 6 colores.\n\n"
                    "- Mecanica de Racha: Las jugadas aplican un multiplicador de 0.2x\n" 
                    " por 4 dots conectados y se resta el multiplicador cuando no se genera esta combinacion.";
            } 
            else if (PaginaInstrucciones == 3) {
                TituloIns.setString("4. ESPECIFICACIONES: Modo Caos");
                TituloIns.setFillColor(sf::Color::Red);
                TextoReglas = 
                    "- Objetivo del Nivel: Alcanzar 400 puntos con 6 colores.\n\n"
                    "- Restricciones: Cuentas con un limite de 15 movimientos.\n\n"
                    "- Dots Negros: Obstaculos indestructibles que bloquean tus cadenas.\n\n"
                    "- Dots Arcoiris: Elementos especiales que actuan como comodines\n"
                    "  perfectos, permitiendo conectar dots y dando un multiplicador de 3.0x en la siguiente jugada.";
            }

            TituloIns.setPosition((800.f - TituloIns.getGlobalBounds().width) / 2.f, 50.f);
            Ventana.draw(TituloIns);

            sf::Text CorpIns(TextoReglas, FuenteJuego, 16);
            CorpIns.setFillColor(sf::Color::Black);
            CorpIns.setPosition(60.f, 140.f);
            Ventana.draw(CorpIns);

            sf::Text VolverIns("Flechas IZQ / DER para cambiar de pagina | Presiona 'Q' o 'ESC' para volver", FuenteJuego, 16);
            VolverIns.setFillColor(sf::Color::Blue);
            VolverIns.setPosition((800.f - VolverIns.getLocalBounds().width) / 2.f, 540.f);
            Ventana.draw(VolverIns);
        }

        // Dibujo de la pantalla de modalidad
        else if (EstadoActual == Modalidad) {
            sf::Text TituloMod("SELECCIONA EL MODO DE JUEGO", FuenteJuego, 32);
            TituloMod.setFillColor(sf::Color::Blue);
            TituloMod.setPosition((800.f - TituloMod.getLocalBounds().width) / 2.f, 100.f);
            Ventana.draw(TituloMod);

            std::string StrModo1 = (SeleccionDificultad == 0) ? "> Modo Lento (Meta: 600 Pts) <" : "Modo Lento (Meta: 600 Pts)";
            sf::Text Modo1(StrModo1, FuenteJuego, 20);
            Modo1.setFillColor((SeleccionDificultad == 0) ? sf::Color(234, 179, 8) : sf::Color::Black);
            Modo1.setPosition((800.f - Modo1.getLocalBounds().width) / 2.f, 230.f);
            Ventana.draw(Modo1);

            std::string StrModo2 = (SeleccionDificultad == 1) ? "> Modo Rapido (Meta: 400 Pts) <" : "Modo Rapido (Meta: 400 Pts)";
            sf::Text Modo2(StrModo2, FuenteJuego, 20);
            Modo2.setFillColor((SeleccionDificultad == 1) ? sf::Color(234, 179, 8) : sf::Color::Black);
            Modo2.setPosition((800.f - Modo2.getLocalBounds().width) / 2.f, 300.f);
            Ventana.draw(Modo2);

            std::string StrModo3 = (SeleccionDificultad == 2) ? "> MODO CAOS (Meta: 400 Pts + Obstaculos) <" : "MODO CAOS (Meta: 400 Pts + Obstaculos)";
            sf::Text Modo3(StrModo3, FuenteJuego, 20);
            Modo3.setFillColor((SeleccionDificultad == 2) ? sf::Color::Red : sf::Color::Black);
            Modo3.setPosition((800.f - Modo3.getLocalBounds().width) / 2.f, 370.f);
            Ventana.draw(Modo3);

            sf::Text TextoPie("Usa las flechas y ENTER | Presiona 'Q' o 'ESC' para regresar", FuenteJuego, 16);
            TextoPie.setFillColor(sf::Color(148, 163, 184));
            TextoPie.setPosition((800.f - TextoPie.getLocalBounds().width) / 2.f, 480.f);
            Ventana.draw(TextoPie);
        }

        // Dibujo de pantalla de carga
        else if (EstadoActual == PantallaCarga) {
            Ventana.clear(sf::Color(240, 244, 248)); 

            sf::Text TxtAviso("PREPARATE...", FuenteJuego, 26);
            TxtAviso.setFillColor(sf::Color(100, 116, 139));
            TxtAviso.setPosition((800.f - TxtAviso.getLocalBounds().width) / 2.f, 180.f);
            Ventana.draw(TxtAviso);

            sf::Text TxtModo("Inicio del  " + NombreModoActual, FuenteJuego, 36);
            TxtModo.setFillColor(ColorModoActual);
            TxtModo.setPosition((800.f - TxtModo.getLocalBounds().width) / 2.f, 240.f);
            Ventana.draw(TxtModo);

            sf::Text TxtObjetivo("¡Consigue " + std::to_string(MetaPuntuacion) + " puntos para ganar!", FuenteJuego, 24);
            TxtObjetivo.setFillColor(sf::Color::Black);
            TxtObjetivo.setPosition((800.f - TxtObjetivo.getLocalBounds().width) / 2.f, 320.f);
            Ventana.draw(TxtObjetivo);

            std::string PuntosDinamicos = "...";
            if ((int)(RelojCarga.getElapsedTime().asSeconds() * 2) % 2 == 0) PuntosDinamicos = " . ";
            sf::Text TxtEspere(PuntosDinamicos, FuenteJuego, 30);
            TxtEspere.setFillColor(ColorModoActual);
            TxtEspere.setPosition((800.f - TxtEspere.getLocalBounds().width) / 2.f, 420.f);
            Ventana.draw(TxtEspere);
        }

        // Dibujo del tablero y de la pantalla de pausa
        else if (EstadoActual == TableroJuego || EstadoActual == Pausa) {
            sf::Text TxtPuntuacion("Pts: " + std::to_string(Puntuacion) + " / " + std::to_string(MetaPuntuacion), FuenteJuego, 22);
            TxtPuntuacion.setFillColor(sf::Color::Blue);
            TxtPuntuacion.setPosition(40, 40);
            Ventana.draw(TxtPuntuacion);

            std::string StrMult = "Multiplicador: " + std::to_string(MultiplicadorEntero / 10) + "." + std::to_string(MultiplicadorEntero % 10) + "x";
            sf::Text TxtMult(StrMult, FuenteJuego, 22);
            
            bool UsaMultiplicador = (ModoSeleccionado == Rapido || ModoSeleccionado == Caos);
            TxtMult.setFillColor(UsaMultiplicador ? (TurnoBonoActivo ? sf::Color::Red : sf::Color(147, 51, 234)) : sf::Color(150, 150, 150)); 
            TxtMult.setPosition((800.f - TxtMult.getLocalBounds().width) / 2.f, 40);
            Ventana.draw(TxtMult);

            sf::Text TxtMovimientos("Mov: " + std::to_string(Movimientos), FuenteJuego, 22);
            TxtMovimientos.setFillColor(sf::Color::Red);
            TxtMovimientos.setPosition(640, 40);
            Ventana.draw(TxtMovimientos);

            JuegoManager.ClonarTablero(TableroCopia);

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
                        case 5: SPunto.setTexture(TMorado); break;  
                        case 6: SPunto.setTexture(TNaranja); break;
                        case 7: SPunto.setTexture(TArcoiris); break; 
                        case 8: SPunto.setTexture(TNegro); break;    
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

            if (EstadoActual == Pausa) {
                sf::RectangleShape CapaFondo(sf::Vector2f(800.f, 600.f));
                CapaFondo.setFillColor(sf::Color(0, 0, 0, 180)); 
                Ventana.draw(CapaFondo);

                sf::Text TxtPausa("JUEGO PAUSADO", FuenteJuego, 36);
                TxtPausa.setFillColor(sf::Color::White);
                TxtPausa.setPosition((800.f - TxtPausa.getLocalBounds().width) / 2.f, 180.f);
                Ventana.draw(TxtPausa);

                std::string StrP1 = (SeleccionPausa == 0) ? "> REANUDAR PARTIDA <" : "REANUDAR PARTIDA";
                sf::Text P1(StrP1, FuenteJuego, 22);
                P1.setFillColor((SeleccionPausa == 0) ? sf::Color::Yellow : sf::Color(200, 200, 200));
                P1.setPosition((800.f - P1.getLocalBounds().width) / 2.f, 290.f);
                Ventana.draw(P1);

                std::string StrP2 = (SeleccionPausa == 1) ? "> SALIR AL MENU PRINCIPAL <" : "SALIR AL MENU PRINCIPAL";
                sf::Text P2(StrP2, FuenteJuego, 22);
                P2.setFillColor((SeleccionPausa == 1) ? sf::Color::Red : sf::Color(200, 200, 200));
                P2.setPosition((800.f - P2.getLocalBounds().width) / 2.f, 360.f);
                Ventana.draw(P2);
            }
        }

        // Dibujo de pantalla final
        else if (EstadoActual == PartidaTerminada) {
            Ventana.clear(sf::Color(255, 255, 255));

            bool Gano = (Puntuacion >= MetaPuntuacion);
            
            sf::Text TextoEstado(Gano ? "VICTORIA!" : "FIN DE LA PARTIDA", FuenteJuego, 40);
            TextoEstado.setFillColor(Gano ? sf::Color::Green : sf::Color::Red);
            TextoEstado.setPosition((800.f - TextoEstado.getLocalBounds().width) / 2.f, 150.f);
            Ventana.draw(TextoEstado);

            sf::Text TextoPuntuacionFinal("Puntuacion Final: " + std::to_string(Puntuacion) + " / " + std::to_string(MetaPuntuacion), FuenteJuego, 28);
            TextoPuntuacionFinal.setFillColor(sf::Color::Black);
            TextoPuntuacionFinal.setPosition((800.f - TextoPuntuacionFinal.getLocalBounds().width) / 2.f, 260.f);
            Ventana.draw(TextoPuntuacionFinal);

            sf::Text TextoReiniciar("Presiona ENTER para volver al menu", FuenteJuego, 18);
            TextoReiniciar.setFillColor(sf::Color::Blue);
            TextoReiniciar.setPosition((800.f - TextoReiniciar.getLocalBounds().width) / 2.f, 420.f);
            Ventana.draw(TextoReiniciar);
        }

        Ventana.display();
    }

    return 0;
}