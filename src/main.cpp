// =====================================================================
//  Calculadora de Matemática Discreta — servidor web
// =====================================================================
//
//  Este archivo solo hace de "mesero":
//    1. Recibe los datos que manda la página web (en formato JSON).
//    2. Llama a la función de cálculo correcta (combinatoria.cpp o grafos.cpp).
//    3. Devuelve el resultado y los pasos a la página, también en JSON.
//
//  Toda la matemática está en combinatoria.cpp y grafos.cpp.

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "httplib.h"   // servidor web (lib/httplib.h)
#include "json.hpp"    // leer y escribir JSON (lib/json.hpp)

#include "combinatoria.h"
#include "grafos.h"

using json = nlohmann::json;
using std::string;
using std::vector;

// =====================================================================
//  Convertir resultados a JSON
// =====================================================================

static json pasosAJson(const vector<Paso> &pasos) {
    json lista = json::array();
    for (const Paso &p : pasos) {
        lista.push_back({{"explicacion", p.explicacion}, {"expresion", p.expresion}});
    }
    return lista;
}

static json conteoAJson(const ResultadoConteo &r) {
    if (!r.ok) return {{"ok", false}, {"error", r.error}};
    return {
        {"ok", true},
        {"formula", r.formula},
        {"simbolo", r.simbolo},
        {"resultado", r.resultado},
        {"pasos", pasosAJson(r.pasos)},
    };
}

static json grafoAJson(const ResultadoGrafo &r) {
    if (!r.ok) return {{"ok", false}, {"error", r.error}};

    json tabla = json::array();
    for (const FilaDijkstra &f : r.tabla) {
        tabla.push_back({{"nodo", f.nodo}, {"distancia", f.distancia}, {"anterior", f.anterior}});
    }
    json caminos = json::array();
    for (const CaminoEncontrado &c : r.caminos) {
        caminos.push_back({{"nodos", c.nodos}, {"pesos", c.pesos}, {"total", c.total}, {"detalle", c.detalle}});
    }
    return {
        {"ok", true},
        {"ruta", r.ruta},
        {"pesos", r.pesos},
        {"total", r.total},
        {"detalle", r.detalle},
        {"pasos", pasosAJson(r.pasos)},
        {"tabla", tabla},
        {"caminos", caminos},
        {"totalCaminos", r.totalCaminos},
    };
}

// Envía la respuesta. Si el cálculo falló, se responde con código 400 (dato inválido).
static void responder(httplib::Response &res, const json &cuerpo) {
    res.status = cuerpo["ok"] == true ? 200 : 400;
    res.set_content(cuerpo.dump(), "application/json; charset=utf-8");
}

static void responderError(httplib::Response &res, const string &mensaje) {
    responder(res, {{"ok", false}, {"error", mensaje}});
}

// =====================================================================
//  Leer datos del JSON que manda la página
// =====================================================================

// Lee un número entero. Si falta o no es entero, escribe el motivo en "error".
static bool leerEntero(const json &datos, const string &campo, long long &valor, string &error) {
    if (!datos.contains(campo) || !datos[campo].is_number_integer()) {
        error = "El valor de " + campo + " debe ser un número entero.";
        return false;
    }
    valor = datos[campo].get<long long>();
    return true;
}

static bool leerTexto(const json &datos, const string &campo, string &valor, string &error) {
    if (!datos.contains(campo) || !datos[campo].is_string()) {
        error = "Falta el dato \"" + campo + "\".";
        return false;
    }
    valor = datos[campo].get<string>();
    return true;
}

// Lee las aristas: [{"origen":"A","destino":"B","peso":5,"dobleSentido":true}, ...]
static bool leerAristas(const json &datos, vector<Arista> &aristas, string &error) {
    if (!datos.contains("aristas") || !datos["aristas"].is_array()) {
        error = "Faltan las aristas del grafo.";
        return false;
    }
    int numero = 0;
    for (const json &a : datos["aristas"]) {
        numero++;
        Arista arista;
        string detalle;
        if (!a.is_object()) {
            detalle = "debe tener origen, destino y peso.";
        } else if (leerTexto(a, "origen", arista.origen, detalle) && leerTexto(a, "destino", arista.destino, detalle) &&
                   leerEntero(a, "peso", arista.peso, detalle)) {
            // "dobleSentido" es opcional; si no viene, la arista es de un solo sentido.
            arista.dobleSentido = false;
            if (a.contains("dobleSentido")) {
                if (!a["dobleSentido"].is_boolean()) {
                    detalle = "el sentido debe ser verdadero o falso.";
                } else {
                    arista.dobleSentido = a["dobleSentido"].get<bool>();
                }
            }
        }
        if (!detalle.empty()) {
            error = "La arista " + std::to_string(numero) + " no es válida: " + detalle;
            return false;
        }
        aristas.push_back(arista);
    }
    return true;
}

// Lee el tipo de grafo: "no-dirigido", "dirigido" o "mixto".
static bool leerTipoGrafo(const json &datos, TipoGrafo &tipo, string &error) {
    string texto;
    if (!leerTexto(datos, "tipo", texto, error)) return false;
    if (texto == "no-dirigido") tipo = NO_DIRIGIDO;
    else if (texto == "dirigido") tipo = DIRIGIDO;
    else if (texto == "mixto") tipo = MIXTO;
    else {
        error = "El tipo de grafo debe ser \"no-dirigido\", \"dirigido\" o \"mixto\".";
        return false;
    }
    return true;
}

// Convierte el texto recibido en JSON. Devuelve false si no es JSON válido.
static bool leerCuerpo(const httplib::Request &req, json &datos) {
    datos = json::parse(req.body, nullptr, false);   // false = no lanzar excepción
    return !datos.is_discarded() && datos.is_object();
}

// =====================================================================
//  Rutas de la API
// =====================================================================

// Las 4 operaciones que reciben n y k se registran igual; solo cambia la función.
using FuncionNK = ResultadoConteo (*)(long long, long long);

static void registrarOperacionNK(httplib::Server &servidor, const string &ruta, FuncionNK funcion) {
    servidor.Post(ruta, [funcion](const httplib::Request &req, httplib::Response &res) {
        json datos;
        long long n, k;
        string error;
        if (!leerCuerpo(req, datos)) return responderError(res, "Los datos enviados no son válidos.");
        if (!leerEntero(datos, "n", n, error) || !leerEntero(datos, "k", k, error)) return responderError(res, error);
        responder(res, conteoAJson(funcion(n, k)));
    });
}

// Permutación con elementos repetidos: recibe una palabra o una lista de cantidades.
static void registrarPermutacionElementos(httplib::Server &servidor) {
    servidor.Post("/api/permutacion-elementos-repetidos", [](const httplib::Request &req, httplib::Response &res) {
        json datos;
        if (!leerCuerpo(req, datos)) return responderError(res, "Los datos enviados no son válidos.");

        if (datos.contains("palabra") && datos["palabra"].is_string()) {
            return responder(res, conteoAJson(permutacionDePalabra(datos["palabra"].get<string>())));
        }
        if (datos.contains("cantidades") && datos["cantidades"].is_array()) {
            vector<long long> cantidades;
            for (const json &c : datos["cantidades"]) {
                if (!c.is_number_integer()) return responderError(res, "Todas las cantidades deben ser números enteros.");
                cantidades.push_back(c.get<long long>());
            }
            return responder(res, conteoAJson(permutacionConCantidades(cantidades)));
        }
        responderError(res, "Escribe una palabra o las cantidades de cada elemento.");
    });
}

// Las 2 operaciones de grafos reciben lo mismo: aristas, tipo de grafo, origen y destino.
using FuncionGrafo = ResultadoGrafo (*)(const vector<Arista> &, TipoGrafo, const string &, const string &);

static void registrarOperacionGrafo(httplib::Server &servidor, const string &ruta, FuncionGrafo funcion) {
    servidor.Post(ruta, [funcion](const httplib::Request &req, httplib::Response &res) {
        json datos;
        vector<Arista> aristas;
        TipoGrafo tipo;
        string origen, destino, error;
        if (!leerCuerpo(req, datos)) return responderError(res, "Los datos enviados no son válidos.");
        if (!leerAristas(datos, aristas, error) || !leerTipoGrafo(datos, tipo, error) ||
            !leerTexto(datos, "origen", origen, error) || !leerTexto(datos, "destino", destino, error)) {
            return responderError(res, error);
        }
        responder(res, grafoAJson(funcion(aristas, tipo, origen, destino)));
    });
}

// =====================================================================
//  Programa principal
// =====================================================================

// Busca la carpeta "web" con la página. Primero en la carpeta actual y, si no
// está ahí, junto al ejecutable (build/../web). Así el programa funciona
// aunque se abra desde otra carpeta. Devuelve "" si no la encuentra.
static string buscarCarpetaWeb(const char *rutaPrograma) {
    namespace fs = std::filesystem;
    std::error_code error;   // para que is_directory no lance excepciones
    if (fs::is_directory("web", error)) return "./web";

    fs::path junto = fs::path(rutaPrograma).parent_path() / ".." / "web";
    if (fs::is_directory(junto, error)) return junto.string();
    return "";
}

int main(int argc, char *argv[]) {
    int puerto = 8080;
    if (argc > 1) puerto = std::atoi(argv[1]);   // ej.: ./calculadora 3000
    if (puerto < 1 || puerto > 65535) {
        std::cerr << "El puerto debe ser un número entre 1 y 65535. Ejemplo: make run PUERTO=3000\n";
        return 1;
    }

    httplib::Server servidor;

    // La carpeta "web" tiene la página (HTML, CSS y JS).
    string carpetaWeb = buscarCarpetaWeb(argv[0]);
    if (carpetaWeb.empty() || !servidor.set_mount_point("/", carpetaWeb)) {
        std::cerr << "No se encontró la carpeta \"web\" con la página.\n"
                  << "Revisa que el proyecto esté completo y ejecútalo desde su carpeta (por ejemplo con: make run)\n";
        return 1;
    }

    registrarOperacionNK(servidor, "/api/permutacion-sin-repeticion", permutacionSinRepeticion);
    registrarOperacionNK(servidor, "/api/permutacion-con-repeticion", permutacionConRepeticion);
    registrarPermutacionElementos(servidor);
    registrarOperacionNK(servidor, "/api/combinacion-sin-repeticion", combinacionSinRepeticion);
    registrarOperacionNK(servidor, "/api/combinacion-con-repeticion", combinacionConRepeticion);
    registrarOperacionGrafo(servidor, "/api/ruta-mas-corta", rutaMasCorta);
    registrarOperacionGrafo(servidor, "/api/ruta-mas-larga", rutaMasLarga);

    // Si algo inesperado falla, la página recibe un mensaje en lugar de quedarse esperando.
    servidor.set_exception_handler([](const httplib::Request &, httplib::Response &res, std::exception_ptr) {
        res.status = 500;
        res.set_content(json({{"ok", false}, {"error", "Ocurrió un error inesperado en el servidor."}}).dump(),
                        "application/json; charset=utf-8");
    });

    // Solo se escucha en esta computadora (127.0.0.1), no en la red.
    if (!servidor.bind_to_port("127.0.0.1", puerto)) {
        std::cerr << "No se pudo usar el puerto " << puerto << ". ¿Ya hay otro programa usándolo?\n"
                  << "Prueba con otro puerto, por ejemplo: make run PUERTO=3000\n";
        return 1;
    }

    std::cout << "\n  Calculadora de Matemática Discreta\n"
              << "  Abre en tu navegador:  http://localhost:" << puerto << "\n"
              << "  Para detener el servidor presiona Ctrl + C\n"
              << std::endl;   // endl fuerza a mostrar el mensaje ya, antes de quedarse esperando

    servidor.listen_after_bind();
    return 0;
}
