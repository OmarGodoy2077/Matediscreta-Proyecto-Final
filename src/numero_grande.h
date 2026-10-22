#ifndef NUMERO_GRANDE_H
#define NUMERO_GRANDE_H

#include <string>
#include <vector>

// Un entero positivo sin límite de dígitos.
//
// ¿Por qué existe? Porque 21! ya no cabe en un "unsigned long long" (el tipo
// entero más grande de C++). Con esta clase los resultados siempre son exactos.
//
// Funciona igual que las operaciones "a mano" de primaria: guarda el número
// dígito por dígito y multiplica o divide llevando el acarreo.
class NumeroGrande {
public:
    NumeroGrande(unsigned long long valor = 0);

    // Multiplica el número por un entero pequeño (ej. 20, 350...).
    void multiplicarPor(unsigned int factor);

    // Divide el número entre un entero pequeño.
    // Solo se usa cuando sabemos que la división es exacta (residuo 0).
    void dividirEntre(unsigned int divisor);

    // Devuelve el número como texto, por ejemplo "3628800".
    std::string texto() const;

private:
    // Los dígitos se guardan al revés: digitos[0] son las unidades,
    // digitos[1] las decenas, etc. Así es más fácil llevar el acarreo.
    std::vector<int> digitos;

    void quitarCerosIzquierda();
};

#endif
