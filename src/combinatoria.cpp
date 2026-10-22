#include "combinatoria.h"

#include <algorithm>
#include <cctype>

#include "numero_grande.h"

using std::string;
using std::to_string;
using std::vector;

// =====================================================================
//  Ayudas para escribir la notación como en clase (Pₖⁿ, Cₖⁿ, 10⁴ ...)
// =====================================================================

static string superindice(long long numero) {
    const char *digitos[] = {"⁰", "¹", "²", "³", "⁴", "⁵", "⁶", "⁷", "⁸", "⁹"};
    string texto;
    for (char c : to_string(numero)) texto += digitos[c - '0'];
    return texto;
}

static string subindice(long long numero) {
    const char *digitos[] = {"₀", "₁", "₂", "₃", "₄", "₅", "₆", "₇", "₈", "₉"};
    string texto;
    for (char c : to_string(numero)) texto += digitos[c - '0'];
    return texto;
}

// simbolo("C", 10, 2) → "C₂¹⁰"
static string simbolo(const string &letra, long long n, long long k) {
    return letra + subindice(k) + superindice(n);
}

// Escribe la multiplicación desde × (desde − 1) × … × hasta.
// Si son muchos factores, se abrevia: "100 × 99 × 98 × … × 2 × 1".
static string productoTexto(long long desde, long long hasta) {
    long long cantidad = desde - hasta + 1;
    string texto;
    for (long long i = desde; i >= hasta; i--) {
        bool esDeLosPrimeros = i > desde - 3;
        bool esDeLosUltimos = i < hasta + 2;
        if (cantidad > 8 && !esDeLosPrimeros && !esDeLosUltimos) {
            if (i == desde - 3) texto += " × …";
            continue;
        }
        if (!texto.empty()) texto += " × ";
        texto += to_string(i);
    }
    return texto;
}

// Escribe n × n × … × n (k veces).
static string potenciaTexto(long long n, long long k) {
    if (k > 8) {
        string base = to_string(n);
        return base + " × " + base + " × " + base + " × … × " + base + "   (" + to_string(k) + " veces)";
    }
    string texto;
    for (long long i = 0; i < k; i++) {
        if (i > 0) texto += " × ";
        texto += to_string(n);
    }
    return texto;
}

// Pone paréntesis solo si hay una multiplicación: "10 × 9" → "(10 × 9)", "10" → "10".
static string entreParentesis(const string &texto) {
    if (texto.find("×") == string::npos) return texto;
    return "(" + texto + ")";
}

static void agregarPaso(ResultadoConteo &r, const string &explicacion, const string &expresion) {
    r.pasos.push_back({explicacion, expresion});
}

// =====================================================================
//  Ayudas de cálculo
// =====================================================================

// Multiplica desde × (desde − 1) × … × hasta. Si no hay factores, da 1.
static NumeroGrande producto(long long desde, long long hasta) {
    NumeroGrande resultado(1);
    for (long long i = desde; i >= hasta; i--) resultado.multiplicarPor((unsigned int)i);
    return resultado;
}

// n! = n × (n − 1) × … × 1   (y 0! = 1)
static NumeroGrande factorial(long long n) {
    return producto(n, 1);
}

// Revisa que un dato esté entre "minimo" y MAXIMO_N.
static bool validarRango(long long valor, const string &nombre, long long minimo, ResultadoConteo &r) {
    if (valor < minimo || valor > MAXIMO_N) {
        r.error = nombre + " debe ser un número entero entre " + to_string(minimo) + " y " + to_string(MAXIMO_N) + ".";
        return false;
    }
    return true;
}

// Resuelve  total! / (a! · b!)  igual que en clase:
//   1. Se expande total! hasta llegar al factorial más grande del denominador.
//   2. Ese factorial se cancela arriba y abajo.
//   3. Se multiplica lo que queda y se divide.
// Ejemplo: 10! / (8! · 2!) = (10 × 9 × 8!) / (8! × 2 × 1) = 90 / 2 = 45
static NumeroGrande resolverCancelando(long long total, long long a, long long b,
                                       const string &sim, ResultadoConteo &r) {
    long long mayor = std::max(a, b);
    long long menor = std::min(a, b);

    if (menor == 0) {
        agregarPaso(r, "Como 0! = 1, arriba y abajo queda lo mismo",
                    sim + " = " + to_string(total) + "! / " + to_string(mayor) + "! = 1");
        return NumeroGrande(1);
    }

    string arriba = productoTexto(total, mayor + 1);
    string abajo = productoTexto(menor, 1);

    agregarPaso(r, "Expandimos " + to_string(total) + "! hasta llegar a " + to_string(mayor) + "!",
                to_string(total) + "! = " + arriba + " × " + to_string(mayor) + "!");
    agregarPaso(r, "Cancelamos " + to_string(mayor) + "! arriba y abajo",
                sim + " = " + entreParentesis(arriba) + " / " + entreParentesis(abajo));

    NumeroGrande numerador = producto(total, mayor + 1);
    NumeroGrande denominador = factorial(menor);
    agregarPaso(r, "Multiplicamos arriba y abajo", sim + " = " + numerador.texto() + " / " + denominador.texto());

    // Dividir entre 2, luego entre 3, ... hasta "menor" es lo mismo que dividir entre menor!.
    // Cada división es exacta porque el producto de "menor" números seguidos
    // siempre es múltiplo de menor!.
    NumeroGrande resultado = numerador;
    for (long long i = 2; i <= menor; i++) resultado.dividirEntre((unsigned int)i);
    agregarPaso(r, "Dividimos", sim + " = " + resultado.texto());
    return resultado;
}

// =====================================================================
//  Permutaciones
// =====================================================================

ResultadoConteo permutacionSinRepeticion(long long n, long long k) {
    ResultadoConteo r;
    r.formula = "Pₖⁿ = n! / (n − k)!";
    if (!validarRango(n, "n", 0, r) || !validarRango(k, "k", 0, r)) return r;
    if (k > n) {
        r.error = "k no puede ser mayor que n: no se pueden ordenar " + to_string(k) +
                  " elementos distintos si solo hay " + to_string(n) + ".";
        return r;
    }

    string sim = simbolo("P", n, k);
    long long resta = n - k;
    r.simbolo = sim;

    agregarPaso(r, "Datos", "n = " + to_string(n) + " (total de elementos),   k = " + to_string(k) +
                                " (se toman de " + to_string(k) + " en " + to_string(k) + ")");
    agregarPaso(r, "Fórmula", r.formula);
    agregarPaso(r, "Sustituimos los valores",
                sim + " = " + to_string(n) + "! / (" + to_string(n) + " − " + to_string(k) + ")! = " +
                    to_string(n) + "! / " + to_string(resta) + "!");

    if (k == 0) {
        agregarPaso(r, "Arriba y abajo queda el mismo factorial", sim + " = 1");
    } else if (resta == 0) {
        agregarPaso(r, "Como 0! = 1, solo queda n!", sim + " = " + productoTexto(n, 1));
    } else {
        string factores = productoTexto(n, resta + 1);
        agregarPaso(r, "Expandimos " + to_string(n) + "! hasta llegar a " + to_string(resta) + "!",
                    to_string(n) + "! = " + factores + " × " + to_string(resta) + "!");
        agregarPaso(r, "Cancelamos " + to_string(resta) + "! arriba y abajo", sim + " = " + factores);
    }

    NumeroGrande resultado = producto(n, resta + 1);
    agregarPaso(r, "Resultado", sim + " = " + resultado.texto());

    r.resultado = resultado.texto();
    r.ok = true;
    return r;
}

ResultadoConteo permutacionConRepeticion(long long n, long long k) {
    ResultadoConteo r;
    r.formula = "PRₖⁿ = nᵏ";
    if (!validarRango(n, "n", 1, r) || !validarRango(k, "k", 0, r)) return r;

    string sim = simbolo("PR", n, k);
    r.simbolo = sim;

    agregarPaso(r, "Datos", "n = " + to_string(n) + " (opciones para cada posición),   k = " + to_string(k) +
                                " (posiciones a llenar)");
    agregarPaso(r, "Fórmula", r.formula);
    agregarPaso(r, "Sustituimos los valores", sim + " = " + to_string(n) + superindice(k));

    if (k == 0) {
        agregarPaso(r, "Todo número elevado a la 0 es 1", sim + " = 1");
    } else {
        agregarPaso(r, "Multiplicamos " + to_string(n) + " por sí mismo " + to_string(k) + " veces",
                    sim + " = " + potenciaTexto(n, k));
    }

    NumeroGrande resultado(1);
    for (long long i = 0; i < k; i++) resultado.multiplicarPor((unsigned int)n);
    agregarPaso(r, "Resultado", sim + " = " + resultado.texto());

    r.resultado = resultado.texto();
    r.ok = true;
    return r;
}

// Un elemento y cuántas veces aparece. Ej.: en "CASA" → {"A", 2}.
struct Grupo {
    string nombre;
    long long cantidad;
};

// Cálculo común de  n! / (n₁! · n₂! · … · nₘ!)
static ResultadoConteo permutacionConGrupos(const vector<Grupo> &grupos, const string &datos) {
    ResultadoConteo r;
    r.formula = "P = n! / (n₁! · n₂! · … · nₘ!)";

    if (grupos.empty()) {
        r.error = "Ingresa al menos un elemento.";
        return r;
    }

    // n es la suma de todas las cantidades.
    long long n = 0;
    string suma;
    for (const Grupo &g : grupos) {
        if (g.cantidad < 1 || g.cantidad > MAXIMO_N) {
            r.error = "Cada cantidad debe ser un número entero entre 1 y " + to_string(MAXIMO_N) + ".";
            return r;
        }
        n += g.cantidad;
        if (!suma.empty()) suma += " + ";
        suma += to_string(g.cantidad);
    }
    if (n > MAXIMO_N) {
        r.error = "El total de elementos no puede pasar de " + to_string(MAXIMO_N) + ".";
        return r;
    }

    string sim = "P" + superindice(n);
    r.simbolo = sim;

    // Armamos los textos "1! · 2! · 1!" y "1 · 2 · 1".
    string factorialesTexto, valoresTexto;
    NumeroGrande denominador(1);
    for (const Grupo &g : grupos) {
        if (!factorialesTexto.empty()) {
            factorialesTexto += " · ";
            valoresTexto += " · ";
        }
        factorialesTexto += to_string(g.cantidad) + "!";
        valoresTexto += factorial(g.cantidad).texto();
        for (long long i = 2; i <= g.cantidad; i++) denominador.multiplicarPor((unsigned int)i);
    }

    NumeroGrande numerador = factorial(n);

    agregarPaso(r, "Datos", datos);
    agregarPaso(r, "Total de elementos", grupos.size() > 1 ? "n = " + suma + " = " + to_string(n) : "n = " + to_string(n));
    agregarPaso(r, "Fórmula", r.formula);
    agregarPaso(r, "Sustituimos los valores", sim + " = " + to_string(n) + "! / (" + factorialesTexto + ")");
    agregarPaso(r, "Calculamos cada factorial", sim + " = " + numerador.texto() + " / (" + valoresTexto + ")");
    agregarPaso(r, "Multiplicamos el denominador", sim + " = " + numerador.texto() + " / " + denominador.texto());

    // Dividimos n! entre cada factorial, un factor a la vez (todas las divisiones son exactas).
    NumeroGrande resultado = numerador;
    for (const Grupo &g : grupos) {
        for (long long i = 2; i <= g.cantidad; i++) resultado.dividirEntre((unsigned int)i);
    }
    agregarPaso(r, "Dividimos", sim + " = " + resultado.texto());

    r.resultado = resultado.texto();
    r.ok = true;
    return r;
}

ResultadoConteo permutacionDePalabra(const string &palabra) {
    // Contamos cuántas veces aparece cada letra, en el orden en que aparecen.
    // Las letras con tilde o la Ñ ocupan más de un byte en UTF-8, por eso se
    // arma cada letra completa antes de contarla.
    vector<Grupo> grupos;
    string palabraLimpia;
    size_t i = 0;
    while (i < palabra.size()) {
        size_t largo = 1;
        while (i + largo < palabra.size() && (palabra[i + largo] & 0xC0) == 0x80) largo++;
        string letra = palabra.substr(i, largo);
        i += largo;

        if (letra == " " || letra == "\t") continue;               // los espacios no cuentan
        if (letra.size() == 1) letra[0] = (char)std::toupper((unsigned char)letra[0]);  // "a" y "A" son iguales
        palabraLimpia += letra;

        bool encontrada = false;
        for (Grupo &g : grupos) {
            if (g.nombre == letra) {
                g.cantidad++;
                encontrada = true;
                break;
            }
        }
        if (!encontrada) grupos.push_back({letra, 1});
    }

    if (grupos.empty()) {
        ResultadoConteo r;
        r.error = "Escribe una palabra.";
        return r;
    }

    string datos = palabraLimpia + "  →  ";
    for (size_t j = 0; j < grupos.size(); j++) {
        if (j > 0) datos += ",  ";
        datos += grupos[j].nombre + " = " + to_string(grupos[j].cantidad);
    }
    return permutacionConGrupos(grupos, datos);
}

ResultadoConteo permutacionConCantidades(const vector<long long> &cantidades) {
    vector<Grupo> grupos;
    string datos;
    for (size_t i = 0; i < cantidades.size(); i++) {
        string nombre = "n" + subindice((long long)i + 1);
        grupos.push_back({nombre, cantidades[i]});
        if (i > 0) datos += ",  ";
        datos += nombre + " = " + to_string(cantidades[i]);
    }
    return permutacionConGrupos(grupos, datos);
}

// =====================================================================
//  Combinaciones
// =====================================================================

ResultadoConteo combinacionSinRepeticion(long long n, long long k) {
    ResultadoConteo r;
    r.formula = "Cₖⁿ = n! / ((n − k)! · k!)";
    if (!validarRango(n, "n", 0, r) || !validarRango(k, "k", 0, r)) return r;
    if (k > n) {
        r.error = "k no puede ser mayor que n: no se pueden elegir " + to_string(k) +
                  " elementos distintos si solo hay " + to_string(n) + ".";
        return r;
    }

    string sim = simbolo("C", n, k);
    r.simbolo = sim;

    agregarPaso(r, "Datos", "n = " + to_string(n) + " (total de elementos),   k = " + to_string(k) +
                                " (se toman de " + to_string(k) + " en " + to_string(k) + ")");
    agregarPaso(r, "Fórmula", r.formula);
    agregarPaso(r, "Sustituimos los valores",
                sim + " = " + to_string(n) + "! / ((" + to_string(n) + " − " + to_string(k) + ")! · " +
                    to_string(k) + "!) = " + to_string(n) + "! / (" + to_string(n - k) + "! · " + to_string(k) + "!)");

    NumeroGrande resultado = resolverCancelando(n, n - k, k, sim, r);

    r.resultado = resultado.texto();
    r.ok = true;
    return r;
}

ResultadoConteo combinacionConRepeticion(long long n, long long k) {
    ResultadoConteo r;
    r.formula = "CRₖⁿ = (n + k − 1)! / (k! · (n − 1)!)";
    if (!validarRango(n, "n", 1, r) || !validarRango(k, "k", 0, r)) return r;

    string sim = simbolo("CR", n, k);
    long long total = n + k - 1;
    r.simbolo = sim;

    agregarPaso(r, "Datos", "n = " + to_string(n) + " (tipos de elementos),   k = " + to_string(k) +
                                " (elementos a elegir, se pueden repetir)");
    agregarPaso(r, "Fórmula", r.formula);
    agregarPaso(r, "Sustituimos los valores",
                sim + " = (" + to_string(n) + " + " + to_string(k) + " − 1)! / (" + to_string(k) + "! · (" +
                    to_string(n) + " − 1)!) = " + to_string(total) + "! / (" + to_string(k) + "! · " +
                    to_string(n - 1) + "!)");

    NumeroGrande resultado = resolverCancelando(total, k, n - 1, sim, r);

    r.resultado = resultado.texto();
    r.ok = true;
    return r;
}
