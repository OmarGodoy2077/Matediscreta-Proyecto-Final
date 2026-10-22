#ifndef PASO_H
#define PASO_H

#include <string>

// Un paso del desarrollo que se le muestra al usuario.
// Ejemplo:
//   explicacion = "Cancelamos 8! arriba y abajo"
//   expresion   = "C₂¹⁰ = (10 × 9) / (2 × 1)"
struct Paso {
    std::string explicacion;
    std::string expresion;
};

#endif
