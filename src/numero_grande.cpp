#include "numero_grande.h"

NumeroGrande::NumeroGrande(unsigned long long valor) {
    // Separamos el número en dígitos, empezando por las unidades.
    do {
        digitos.push_back(valor % 10);
        valor /= 10;
    } while (valor > 0);
}

void NumeroGrande::multiplicarPor(unsigned int factor) {
    // Multiplicación de primaria: cada dígito por el factor, más lo que "llevamos".
    unsigned long long llevamos = 0;
    for (int &digito : digitos) {
        unsigned long long producto = (unsigned long long)digito * factor + llevamos;
        digito = producto % 10;
        llevamos = producto / 10;
    }
    // Lo que sobró del acarreo se agrega como dígitos nuevos a la izquierda.
    while (llevamos > 0) {
        digitos.push_back(llevamos % 10);
        llevamos /= 10;
    }
    quitarCerosIzquierda();
}

void NumeroGrande::dividirEntre(unsigned int divisor) {
    // División larga de primaria: se recorre desde el dígito de más a la izquierda.
    unsigned long long residuo = 0;
    for (int i = (int)digitos.size() - 1; i >= 0; i--) {
        unsigned long long actual = residuo * 10 + digitos[i];
        digitos[i] = actual / divisor;
        residuo = actual % divisor;
    }
    quitarCerosIzquierda();
}

std::string NumeroGrande::texto() const {
    std::string resultado;
    // Como están guardados al revés, los leemos del último al primero.
    for (int i = (int)digitos.size() - 1; i >= 0; i--) {
        resultado += (char)('0' + digitos[i]);
    }
    return resultado;
}

void NumeroGrande::quitarCerosIzquierda() {
    // Siempre dejamos al menos un dígito (el número 0 es "0", no "").
    while (digitos.size() > 1 && digitos.back() == 0) {
        digitos.pop_back();
    }
}
