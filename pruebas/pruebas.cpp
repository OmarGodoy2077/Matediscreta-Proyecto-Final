// =====================================================================
//  Pruebas automáticas
// =====================================================================
//  Comprueba que los cálculos den los mismos resultados que los ejemplos
//  vistos en clase y otros valores conocidos.
//  Se ejecutan con:  make test

#include <iostream>
#include <string>
#include <vector>

#include "combinatoria.h"
#include "grafos.h"

using std::string;

static int aprobadas = 0;
static int fallidas = 0;

static void comprobar(const string &nombre, const string &obtenido, const string &esperado) {
    if (obtenido == esperado) {
        aprobadas++;
        std::cout << "  [OK]    " << nombre << " = " << obtenido << "\n";
    } else {
        fallidas++;
        std::cout << "  [FALLA] " << nombre << ": se esperaba " << esperado << " y se obtuvo " << obtenido << "\n";
    }
}

static void comprobarError(const string &nombre, bool ok) {
    comprobar(nombre + " (debe rechazarse)", ok ? "aceptado" : "rechazado", "rechazado");
}

static string unirRuta(const std::vector<string> &nodos) {
    string texto;
    for (const string &n : nodos) texto += (texto.empty() ? "" : "-") + n;
    return texto;
}

int main() {
    std::cout << "\nPermutaciones\n";
    comprobar("P(3,2) premios de pintura (clase 1)", permutacionSinRepeticion(3, 2).resultado, "6");
    comprobar("P(4,3) sillas de colores (clase 1)", permutacionSinRepeticion(4, 3).resultado, "24");
    comprobar("P(25,25) = 25!", permutacionSinRepeticion(25, 25).resultado, "15511210043330985984000000");
    comprobar("P(5,0)", permutacionSinRepeticion(5, 0).resultado, "1");
    comprobarError("P(2,5)", permutacionSinRepeticion(2, 5).ok);
    comprobar("PR(10,4) PIN de 4 dígitos", permutacionConRepeticion(10, 4).resultado, "10000");
    comprobar("PR(2,10)", permutacionConRepeticion(2, 10).resultado, "1024");
    comprobar("Anagramas de CASA", permutacionDePalabra("CASA").resultado, "12");
    comprobar("Anagramas de MISSISSIPPI", permutacionDePalabra("mississippi").resultado, "34650");
    comprobar("Anagramas de AÑO (letra Ñ)", permutacionDePalabra("AÑO").resultado, "6");
    comprobar("Cantidades {2,1,1}", permutacionConCantidades({2, 1, 1}).resultado, "12");
    comprobarError("Cantidades {0,2}", permutacionConCantidades({0, 2}).ok);

    std::cout << "\nCombinaciones\n";
    comprobar("C(4,2) ensalada (clase 1)", combinacionSinRepeticion(4, 2).resultado, "6");
    comprobar("C(10,2) torneo de ajedrez (clase 1)", combinacionSinRepeticion(10, 2).resultado, "45");
    comprobar("C(52,5) manos de póker", combinacionSinRepeticion(52, 5).resultado, "2598960");
    comprobar("C(100,50)", combinacionSinRepeticion(100, 50).resultado, "100891344545564193334812497256");
    comprobar("C(7,7)", combinacionSinRepeticion(7, 7).resultado, "1");
    comprobarError("C(3,4)", combinacionSinRepeticion(3, 4).ok);
    comprobar("CR(4,3)", combinacionConRepeticion(4, 3).resultado, "20");
    comprobar("CR(3,2)", combinacionConRepeticion(3, 2).resultado, "6");
    comprobarError("CR(0,3)", combinacionConRepeticion(0, 3).ok);

    std::cout << "\nGrafo mixto (ejemplo de las casas, clase 4)\n";
    std::vector<Arista> casas = {
        {"1", "2", 5, false}, {"1", "3", 7, true}, {"2", "3", 6, false}, {"2", "4", 2, true},
        {"3", "5", 4, false}, {"4", "5", 3, true}, {"5", "6", 2, true},  {"5", "7", 5, true},
    };
    ResultadoGrafo corta = rutaMasCorta(casas, MIXTO, "1", "5");
    comprobar("Ruta más corta 1→5", unirRuta(corta.ruta), "1-2-4-5");
    comprobar("Distancia más corta 1→5", std::to_string(corta.total), "10");

    ResultadoGrafo larga = rutaMasLarga(casas, MIXTO, "1", "5");
    comprobar("Ruta más larga 1→5", unirRuta(larga.ruta), "1-2-3-5");
    comprobar("Distancia más larga 1→5", std::to_string(larga.total), "15");
    comprobar("Caminos simples 1→5", std::to_string(larga.totalCaminos), "3");

    // 7 → 1: se puede volver porque 5↔4↔2 y 3↔1 son de doble sentido, pero 2→1 no.
    ResultadoGrafo regreso = rutaMasCorta(casas, MIXTO, "7", "1");
    comprobar("Ruta más corta 7→1", unirRuta(regreso.ruta), "7-5-4-2-3-1");

    std::cout << "\nGrafo dirigido (mismas aristas: todas van de origen a destino)\n";
    comprobar("Ruta más corta 1→5", unirRuta(rutaMasCorta(casas, DIRIGIDO, "1", "5").ruta), "1-2-4-5");
    comprobar("Caminos simples 1→5", std::to_string(rutaMasLarga(casas, DIRIGIDO, "1", "5").totalCaminos), "3");
    comprobarError("Ruta 5→4 (la arista 4→5 solo va de ida)", rutaMasCorta(casas, DIRIGIDO, "5", "4").ok);
    comprobarError("Ruta 7→1 (no se puede regresar)", rutaMasCorta(casas, DIRIGIDO, "7", "1").ok);
    comprobarError("Ignora ↔ en un grafo dirigido", rutaMasCorta({{"A", "B", 4, true}}, DIRIGIDO, "B", "A").ok);

    std::cout << "\nGrafo no dirigido (mismas aristas: todas van y vienen)\n";
    ResultadoGrafo sinDireccion = rutaMasCorta(casas, NO_DIRIGIDO, "7", "1");
    comprobar("Ruta más corta 7→1", unirRuta(sinDireccion.ruta), "7-5-4-2-1");
    comprobar("Distancia más corta 7→1", std::to_string(sinDireccion.total), "15");
    ResultadoGrafo largaSinDireccion = rutaMasLarga(casas, NO_DIRIGIDO, "1", "5");
    comprobar("Ruta más larga 1→5", unirRuta(largaSinDireccion.ruta), "1-3-2-4-5");
    comprobar("Distancia más larga 1→5", std::to_string(largaSinDireccion.total), "18");
    comprobar("Ignora → en un grafo no dirigido",
              std::to_string(rutaMasCorta({{"A", "B", 4, false}}, NO_DIRIGIDO, "B", "A").total), "4");
    comprobarError("Nodos sin conexión", rutaMasCorta({{"A", "B", 1, true}, {"C", "D", 1, true}}, NO_DIRIGIDO, "A", "D").ok);

    std::cout << "\nErrores\n";
    std::vector<Arista> soloIda = {{"A", "B", 4, false}};
    comprobarError("Ruta B→A con arista de un solo sentido", rutaMasCorta(soloIda, MIXTO, "B", "A").ok);
    comprobarError("Peso negativo", rutaMasCorta({{"A", "B", -1, true}}, NO_DIRIGIDO, "A", "B").ok);
    comprobarError("Nodo inexistente", rutaMasCorta(soloIda, MIXTO, "A", "Z").ok);

    std::cout << "\nResultado: " << aprobadas << " aprobadas, " << fallidas << " fallidas\n\n";
    return fallidas == 0 ? 0 : 1;
}
