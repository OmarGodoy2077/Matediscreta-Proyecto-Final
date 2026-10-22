#include "grafos.h"

#include <algorithm>
#include <climits>

using std::string;
using std::to_string;
using std::vector;

// =====================================================================
//  Representación interna del grafo: lista de adyacencia
// =====================================================================

// "Desde el nodo actual puedo ir a 'destino' y me cuesta 'peso'".
struct Conexion {
    int destino;
    long long peso;
};

// Los nodos se numeran 0, 1, 2... en el orden en que aparecen.
// nombres[i]  → el nombre que escribió el usuario (ej. "A" o "Casa 1")
// vecinos[i]  → a qué nodos se puede ir desde el nodo i
// Una arista de doble sentido se guarda dos veces: a → b y b → a.
struct Grafo {
    vector<string> nombres;
    vector<vector<Conexion>> vecinos;
};

const long long INFINITO = LLONG_MAX;
const long long MAXIMO_LLAMADAS = 5000000;   // freno para la búsqueda de la ruta más larga
const size_t MAXIMO_CAMINOS = 50000;         // caminos que se pueden guardar como máximo
const size_t CAMINOS_A_MOSTRAR = 50;         // caminos que se devuelven para la tabla

// =====================================================================
//  Ayudas
// =====================================================================

static void agregarPaso(ResultadoGrafo &r, const string &explicacion, const string &expresion) {
    r.pasos.push_back({explicacion, expresion});
}

// Quita espacios al inicio y al final: "  A " → "A"
static string limpiar(const string &texto) {
    size_t inicio = texto.find_first_not_of(" \t");
    if (inicio == string::npos) return "";
    size_t fin = texto.find_last_not_of(" \t");
    return texto.substr(inicio, fin - inicio + 1);
}

static int buscarNodo(const Grafo &g, const string &nombre) {
    for (size_t i = 0; i < g.nombres.size(); i++) {
        if (g.nombres[i] == nombre) return (int)i;
    }
    return -1;
}

// Devuelve el número del nodo; si no existe todavía, lo crea.
static int agregarNodo(Grafo &g, const string &nombre) {
    int indice = buscarNodo(g, nombre);
    if (indice != -1) return indice;
    g.nombres.push_back(nombre);
    g.vecinos.push_back({});
    return (int)g.nombres.size() - 1;
}

static string textoDistancia(long long distancia) {
    return distancia == INFINITO ? "∞" : to_string(distancia);
}

// {"1", "2", "4"} → "1 → 2 → 4"
static string rutaTexto(const vector<string> &nodos) {
    string texto;
    for (size_t i = 0; i < nodos.size(); i++) {
        if (i > 0) texto += " → ";
        texto += nodos[i];
    }
    return texto;
}

// {5, 2, 3} → "5 + 2 + 3 = 10"
static string sumaTexto(const vector<long long> &pesos, long long total) {
    string texto;
    for (size_t i = 0; i < pesos.size(); i++) {
        if (i > 0) texto += " + ";
        texto += to_string(pesos[i]);
    }
    if (pesos.size() > 1) texto += " = " + to_string(total);
    return texto;
}

// ¿Esta arista también se puede recorrer de regreso (de destino a origen)?
static bool esDobleSentido(const Arista &arista, TipoGrafo tipo) {
    if (tipo == NO_DIRIGIDO) return true;    // no dirigido: todas van y vienen
    if (tipo == DIRIGIDO) return false;      // dirigido: todas van en un solo sentido
    return arista.dobleSentido;              // mixto: lo decide cada arista
}

// Texto que explica el tipo de grafo en el primer paso del desarrollo.
static string textoTipo(TipoGrafo tipo) {
    if (tipo == NO_DIRIGIDO) return "Grafo no dirigido: cada arista se recorre en ambos sentidos (A — B)";
    if (tipo == DIRIGIDO) return "Grafo dirigido: cada arista solo se recorre de origen a destino (A → B)";
    return "Grafo mixto: las aristas → van en un solo sentido y las ↔ en ambos";
}

// Mensaje cuando el destino no se puede alcanzar desde el origen.
static string textoSinRuta(const Grafo &g, int inicio, int fin, TipoGrafo tipo) {
    string texto = "No existe ninguna ruta de " + g.nombres[inicio] + " a " + g.nombres[fin] + ". ";
    if (tipo == NO_DIRIGIDO) return texto + "Ninguna cadena de aristas une esos dos nodos.";
    return texto + "Revisa el sentido de las aristas (→ solo permite ir en una dirección).";
}

// Convierte la lista de aristas del usuario en un Grafo y revisa que todo sea válido.
// Si algo está mal, devuelve false y escribe el motivo en "error".
static bool construirGrafo(const vector<Arista> &aristas, TipoGrafo tipo, const string &origen, const string &destino,
                           Grafo &g, int &inicio, int &fin, string &error) {
    if (aristas.empty()) {
        error = "Agrega al menos una arista.";
        return false;
    }
    if ((int)aristas.size() > MAXIMO_ARISTAS) {
        error = "Se permiten como máximo " + to_string(MAXIMO_ARISTAS) + " aristas.";
        return false;
    }

    for (size_t i = 0; i < aristas.size(); i++) {
        string numero = to_string(i + 1);
        string desde = limpiar(aristas[i].origen);
        string hasta = limpiar(aristas[i].destino);

        if (desde.empty() || hasta.empty()) {
            error = "A la arista " + numero + " le falta el nodo de origen o de destino.";
            return false;
        }
        if (desde.size() > 20 || hasta.size() > 20) {
            error = "El nombre de un nodo puede tener como máximo 20 caracteres (arista " + numero + ").";
            return false;
        }
        if (aristas[i].peso < 0 || aristas[i].peso > MAXIMO_PESO) {
            error = "El peso de la arista " + numero + " (" + desde + " – " + hasta +
                    ") debe ser un entero entre 0 y " + to_string(MAXIMO_PESO) + ".";
            return false;
        }

        int a = agregarNodo(g, desde);
        int b = agregarNodo(g, hasta);
        g.vecinos[a].push_back({b, aristas[i].peso});                                  // ida: a → b
        if (esDobleSentido(aristas[i], tipo)) g.vecinos[b].push_back({a, aristas[i].peso});   // regreso: b → a
    }

    if ((int)g.nombres.size() > MAXIMO_NODOS) {
        error = "El grafo tiene " + to_string(g.nombres.size()) + " nodos; el máximo es " + to_string(MAXIMO_NODOS) + ".";
        return false;
    }

    inicio = buscarNodo(g, limpiar(origen));
    fin = buscarNodo(g, limpiar(destino));
    if (inicio == -1) {
        error = "El nodo de origen \"" + limpiar(origen) + "\" no aparece en ninguna arista.";
        return false;
    }
    if (fin == -1) {
        error = "El nodo de destino \"" + limpiar(destino) + "\" no aparece en ninguna arista.";
        return false;
    }
    if (inicio == fin) {
        error = "El origen y el destino deben ser nodos distintos.";
        return false;
    }
    return true;
}

// =====================================================================
//  Ruta más corta: algoritmo de Dijkstra
// =====================================================================
//
//  Idea: empezamos en el origen con distancia 0 y todos los demás en ∞.
//  En cada vuelta:
//    1. Tomamos el nodo NO visitado con la menor distancia. Esa distancia ya
//       es definitiva (con pesos ≥ 0 no existe un atajo más barato).
//    2. Revisamos sus vecinos: si llegar a ellos pasando por este nodo es más
//       barato que lo que teníamos, actualizamos su distancia y recordamos
//       de dónde venimos ("anterior").
//  Al final, la ruta se reconstruye caminando hacia atrás con "anterior".

ResultadoGrafo rutaMasCorta(const vector<Arista> &aristas, TipoGrafo tipo, const string &origen, const string &destino) {
    ResultadoGrafo r;
    Grafo g;
    int inicio, fin;
    if (!construirGrafo(aristas, tipo, origen, destino, g, inicio, fin, r.error)) return r;

    int cantidad = (int)g.nombres.size();
    vector<long long> distancia(cantidad, INFINITO);
    vector<int> anterior(cantidad, -1);
    vector<long long> pesoDesdeAnterior(cantidad, 0);
    vector<bool> visitado(cantidad, false);
    distancia[inicio] = 0;

    agregarPaso(r, "Tipo de grafo", textoTipo(tipo));
    agregarPaso(r, "Inicio: el origen empieza en 0 y los demás nodos en ∞",
                "d(" + g.nombres[inicio] + ") = 0");

    for (int vuelta = 1; vuelta <= cantidad; vuelta++) {
        // 1. Buscar el nodo no visitado con la menor distancia.
        int actual = -1;
        for (int i = 0; i < cantidad; i++) {
            if (visitado[i] || distancia[i] == INFINITO) continue;
            if (actual == -1 || distancia[i] < distancia[actual]) actual = i;
        }
        if (actual == -1) break;   // los nodos que faltan no se pueden alcanzar
        visitado[actual] = true;

        // 2. Revisar los vecinos del nodo actual.
        string cambios;
        for (const Conexion &c : g.vecinos[actual]) {
            if (visitado[c.destino]) continue;
            long long nuevaDistancia = distancia[actual] + c.peso;
            if (nuevaDistancia < distancia[c.destino]) {
                if (!cambios.empty()) cambios += ",   ";
                cambios += "d(" + g.nombres[c.destino] + "): " + textoDistancia(distancia[c.destino]) + " → " +
                           to_string(distancia[actual]) + " + " + to_string(c.peso) + " = " + to_string(nuevaDistancia);
                distancia[c.destino] = nuevaDistancia;
                anterior[c.destino] = actual;
                pesoDesdeAnterior[c.destino] = c.peso;
            }
        }
        if (cambios.empty()) cambios = "Ningún vecino mejora su distancia";

        agregarPaso(r, "Vuelta " + to_string(vuelta) + ": visitamos " + g.nombres[actual] +
                           " (distancia definitiva " + to_string(distancia[actual]) + ")",
                    cambios);
    }

    // Tabla final: distancia y nodo anterior de cada nodo.
    for (int i = 0; i < cantidad; i++) {
        r.tabla.push_back({g.nombres[i], textoDistancia(distancia[i]),
                           anterior[i] == -1 ? "—" : g.nombres[anterior[i]]});
    }

    if (distancia[fin] == INFINITO) {
        r.error = textoSinRuta(g, inicio, fin, tipo);
        return r;
    }

    // 3. Reconstruir la ruta: desde el destino vamos hacia atrás usando "anterior".
    vector<int> camino;
    for (int nodo = fin; nodo != -1; nodo = anterior[nodo]) camino.push_back(nodo);
    std::reverse(camino.begin(), camino.end());

    vector<long long> pesos;
    string haciaAtras;
    for (size_t i = 0; i < camino.size(); i++) {
        r.ruta.push_back(g.nombres[camino[i]]);
        if (i > 0) pesos.push_back(pesoDesdeAnterior[camino[i]]);
    }
    for (int i = (int)camino.size() - 1; i >= 0; i--) {
        haciaAtras += g.nombres[camino[i]];
        if (i > 0) haciaAtras += " ← ";
    }

    r.pesos = pesos;
    r.total = distancia[fin];
    r.detalle = sumaTexto(pesos, r.total);
    agregarPaso(r, "Reconstruimos la ruta desde el destino siguiendo la columna \"anterior\"", haciaAtras);
    agregarPaso(r, "Ruta más corta", rutaTexto(r.ruta) + "   =   " + r.detalle);
    r.ok = true;
    return r;
}

// =====================================================================
//  Ruta más larga: búsqueda en profundidad (DFS) con retroceso
// =====================================================================
//
//  La ruta más larga debe ser un camino SIMPLE (sin repetir nodos); si no,
//  con un ciclo podríamos dar vueltas para siempre y la ruta sería infinita.
//
//  No existe un algoritmo rápido como Dijkstra para este problema (es
//  NP-difícil), así que la forma segura y exacta es probar TODOS los caminos
//  simples y quedarnos con el de mayor peso. Con grafos pequeños como los de
//  clase esto es instantáneo.

// Todo lo que la búsqueda va recordando mientras avanza.
struct Busqueda {
    int destino;
    vector<bool> visitado;
    vector<int> caminoActual;          // nodos del camino que estamos probando
    vector<long long> pesosActuales;   // pesos de las aristas de ese camino
    long long pesoActual = 0;
    vector<CaminoEncontrado> encontrados;
    long long llamadas = 0;
    bool detenida = false;             // true si el grafo tiene demasiados caminos
};

static void explorar(const Grafo &g, int actual, Busqueda &b) {
    b.llamadas++;
    if (b.llamadas > MAXIMO_LLAMADAS || b.encontrados.size() >= MAXIMO_CAMINOS) {
        b.detenida = true;
        return;
    }

    // ¿Llegamos al destino? Guardamos este camino y regresamos.
    if (actual == b.destino) {
        vector<string> nodos;
        for (int n : b.caminoActual) nodos.push_back(g.nombres[n]);
        b.encontrados.push_back({nodos, b.pesosActuales, b.pesoActual, sumaTexto(b.pesosActuales, b.pesoActual)});
        return;
    }

    for (const Conexion &c : g.vecinos[actual]) {
        if (b.visitado[c.destino]) continue;   // no repetir nodos

        // Avanzamos al vecino...
        b.visitado[c.destino] = true;
        b.caminoActual.push_back(c.destino);
        b.pesosActuales.push_back(c.peso);
        b.pesoActual += c.peso;

        explorar(g, c.destino, b);

        // ...y retrocedemos (backtracking) para poder probar el siguiente vecino.
        b.visitado[c.destino] = false;
        b.caminoActual.pop_back();
        b.pesosActuales.pop_back();
        b.pesoActual -= c.peso;

        if (b.detenida) return;
    }
}

ResultadoGrafo rutaMasLarga(const vector<Arista> &aristas, TipoGrafo tipo, const string &origen, const string &destino) {
    ResultadoGrafo r;
    Grafo g;
    int inicio, fin;
    if (!construirGrafo(aristas, tipo, origen, destino, g, inicio, fin, r.error)) return r;

    Busqueda b;
    b.destino = fin;
    b.visitado.assign(g.nombres.size(), false);
    b.visitado[inicio] = true;
    b.caminoActual.push_back(inicio);
    explorar(g, inicio, b);

    if (b.detenida) {
        r.error = "El grafo tiene demasiados caminos posibles para revisarlos todos. Usa menos nodos o aristas.";
        return r;
    }
    if (b.encontrados.empty()) {
        r.error = textoSinRuta(g, inicio, fin, tipo);
        return r;
    }

    // Ordenamos de mayor a menor peso total. "stable_sort" respeta el orden en
    // que se encontraron los caminos cuando hay empates.
    std::stable_sort(b.encontrados.begin(), b.encontrados.end(),
                     [](const CaminoEncontrado &x, const CaminoEncontrado &y) { return x.total > y.total; });

    const CaminoEncontrado &mayor = b.encontrados[0];
    int empatados = 0;
    for (const CaminoEncontrado &c : b.encontrados) {
        if (c.total == mayor.total) empatados++;
    }

    r.totalCaminos = (long long)b.encontrados.size();
    r.ruta = mayor.nodos;
    r.pesos = mayor.pesos;
    r.total = mayor.total;
    r.detalle = mayor.detalle;
    for (size_t i = 0; i < b.encontrados.size() && i < CAMINOS_A_MOSTRAR; i++) {
        r.caminos.push_back(b.encontrados[i]);
    }

    agregarPaso(r, "Tipo de grafo", textoTipo(tipo));
    agregarPaso(r, "Idea: probar todos los caminos que no repiten nodos (DFS con retroceso)",
                "Desde " + g.nombres[inicio] + " hasta " + g.nombres[fin]);
    agregarPaso(r, "Caminos encontrados", to_string(r.totalCaminos) + (r.totalCaminos == 1 ? " camino" : " caminos distintos"));
    agregarPaso(r, "Comparamos el peso total de cada camino y elegimos el mayor",
                "Mayor = " + to_string(mayor.total) + ",   menor = " + to_string(b.encontrados.back().total));
    if (empatados > 1) {
        agregarPaso(r, "Hay un empate", to_string(empatados) + " caminos suman " + to_string(mayor.total) +
                                             "; se muestra el primero que se encontró");
    }
    agregarPaso(r, "Ruta más larga", rutaTexto(r.ruta) + "   =   " + r.detalle);
    r.ok = true;
    return r;
}
