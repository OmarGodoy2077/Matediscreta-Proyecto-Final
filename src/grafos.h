#ifndef GRAFOS_H
#define GRAFOS_H

#include <string>
#include <vector>

#include "paso.h"

// Tipo de grafo que elige el usuario.
//   NO_DIRIGIDO → todas las aristas se recorren en ambos sentidos   (A — B)
//   DIRIGIDO    → todas las aristas van solo de origen a destino    (A → B)
//   MIXTO       → cada arista dice su propio sentido con "dobleSentido"
enum TipoGrafo { NO_DIRIGIDO, DIRIGIDO, MIXTO };

// Una arista tal como la escribe el usuario.
//   dobleSentido = false  →  solo se puede ir de origen a destino   (A → B)
//   dobleSentido = true   →  se puede ir en ambos sentidos          (A ↔ B)
// "dobleSentido" solo se toma en cuenta en un grafo MIXTO.
struct Arista {
    std::string origen;
    std::string destino;
    long long peso;
    bool dobleSentido;
};

// Una fila de la tabla final de Dijkstra.
struct FilaDijkstra {
    std::string nodo;
    std::string distancia;   // "∞" si no se puede llegar
    std::string anterior;    // de qué nodo venimos ("—" si es el origen o no se llega)
};

// Un camino encontrado por la búsqueda de la ruta más larga.
struct CaminoEncontrado {
    std::vector<std::string> nodos;
    std::vector<long long> pesos;   // peso de cada arista usada, en orden
    long long total;
    std::string detalle;            // ej. "5 + 6 + 4 = 15"
};

// Respuesta de la ruta más corta o más larga.
struct ResultadoGrafo {
    bool ok = false;
    std::string error;
    std::vector<std::string> ruta;            // ej. {"1", "2", "4", "5"}
    std::vector<long long> pesos;             // peso de cada arista de la ruta, ej. {5, 2, 3}
    long long total = 0;                      // peso total de la ruta
    std::string detalle;                      // ej. "5 + 2 + 3 = 10"
    std::vector<Paso> pasos;
    std::vector<FilaDijkstra> tabla;          // solo en la ruta más corta
    std::vector<CaminoEncontrado> caminos;    // solo en la ruta más larga (ordenados de mayor a menor)
    long long totalCaminos = 0;               // cuántos caminos se encontraron en total
};

// Límites para que el programa siempre responda rápido.
const int MAXIMO_NODOS = 15;
const int MAXIMO_ARISTAS = 60;
const long long MAXIMO_PESO = 1000000;

// Ruta más corta con el algoritmo de Dijkstra.
ResultadoGrafo rutaMasCorta(const std::vector<Arista> &aristas, TipoGrafo tipo, const std::string &origen,
                            const std::string &destino);

// Ruta más larga (camino simple: sin repetir nodos) probando todos los caminos con DFS.
ResultadoGrafo rutaMasLarga(const std::vector<Arista> &aristas, TipoGrafo tipo, const std::string &origen,
                            const std::string &destino);

#endif
