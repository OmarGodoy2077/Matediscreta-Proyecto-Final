#ifndef COMBINATORIA_H
#define COMBINATORIA_H

#include <string>
#include <vector>

#include "paso.h"

// Respuesta de cualquier cálculo de permutaciones o combinaciones.
struct ResultadoConteo {
    bool ok = false;           // true si se pudo calcular
    std::string error;         // mensaje para el usuario cuando ok es false
    std::string formula;       // fórmula general, ej. "Cₖⁿ = n! / ((n − k)! · k!)"
    std::string simbolo;       // fórmula con los valores del usuario, ej. "C₂¹⁰"
    std::string resultado;     // el número final como texto (puede ser enorme)
    std::vector<Paso> pasos;   // desarrollo paso a paso
};

// Valor máximo permitido para n y k (los resultados siguen siendo exactos).
const long long MAXIMO_N = 1000;

// ---------------- Permutaciones (el orden SÍ importa) ----------------

// Sin repetición: Pₖⁿ = n! / (n − k)!
ResultadoConteo permutacionSinRepeticion(long long n, long long k);

// Con repetición (cada posición puede usar cualquier elemento): PRₖⁿ = nᵏ
ResultadoConteo permutacionConRepeticion(long long n, long long k);

// Con elementos repetidos, a partir de una palabra: ej. "CASA" → 4! / 2! = 12
ResultadoConteo permutacionDePalabra(const std::string &palabra);

// Con elementos repetidos, a partir de cuántas veces se repite cada uno:
// ej. {2, 1, 1} → 4! / (2! · 1! · 1!) = 12
ResultadoConteo permutacionConCantidades(const std::vector<long long> &cantidades);

// ---------------- Combinaciones (el orden NO importa) ----------------

// Sin repetición: Cₖⁿ = n! / ((n − k)! · k!)
ResultadoConteo combinacionSinRepeticion(long long n, long long k);

// Con repetición: CRₖⁿ = (n + k − 1)! / (k! · (n − 1)!)
ResultadoConteo combinacionConRepeticion(long long n, long long k);

#endif
