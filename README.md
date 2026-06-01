# Dots Game - Proyecto 02 Fase 2

Este es un videojuego Dots desarrollado en **C++** utilizando la librería gráfica **SFML**. El proyecto incluye mecánicas similares al juego real, además de renderizado de imágenes procesadas y texto dinámico.

## Integrantes
* **Diego Ayala**
* **David Berganza**
* **Wilfred Orellana**

## Requisitos del Sistema
Para compilar y ejecutar este juego en Linux (Fedora/Ubuntu), se necesita tener instaladas las herramientas de desarrollo de C++ y las dependencias de SFML:

```bash
## En Fedora:
sudo dnf install SFML-devel gcc-c++

## En WLS Ubuntu
sudo apt update && sudo apt install libsfml-dev g++

## Compilar
g++ main.cpp -o dots_visual -lsfml-graphics -lsfml-window -lsfml-system -lpthread

## Ejecutar
./dots_modular

## Controles
Flechas Direccionales: Navegar por los menús y mover el cursor en el tablero.

Barra Espaciadora: Seleccionar un punto / Deseleccionar el último punto de la cadena actual.

Enter: Confirmar la selección en los menús / Validar y eliminar la cadena de puntos en juego.

Q: Regresar al menú principal.