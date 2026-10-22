# Cómo funciona el proyecto

## 1. Vista general

La aplicación tiene dos partes que se comunican por HTTP, igual que cualquier página web:

```
┌──────────────────────────┐   1. POST /api/...   ┌─────────────────────────────────┐
│  Navegador               │   {"n":10,"k":2}     │  Servidor C++ (main.cpp)        │
│  index.html · app.js     │ ───────────────────▶ │  lee el JSON y valida los tipos │
│                          │                      │            │                    │
│  - menú y formularios    │                      │            ▼ 2. llama           │
│  - dibuja el grafo (SVG) │                      │  combinatoria.cpp / grafos.cpp  │
│  - muestra los pasos     │ ◀─────────────────── │  calcula el resultado + pasos   │
└──────────────────────────┘   3. respuesta JSON  └─────────────────────────────────┘
                               {"ok":true,"resultado":"45","pasos":[…]}
```

1. El usuario llena un formulario y presiona **Calcular**. `app.js` envía los datos al servidor con `fetch`.
2. `main.cpp` convierte el JSON en variables de C++ y llama a la función de cálculo.
3. La función devuelve el resultado, los pasos del desarrollo o un mensaje de error. `main.cpp` lo convierte a JSON y `app.js` lo muestra.

**Regla del proyecto:** JavaScript nunca calcula. Solo muestra lo que devuelve C++.

---

## 2. API del servidor

Todas las rutas reciben y devuelven JSON con el método `POST`. Si el dato es inválido, la respuesta tiene código **400** y la forma `{"ok": false, "error": "mensaje para el usuario"}`.

| Ruta | Datos que recibe |
|---|---|
| `/api/permutacion-sin-repeticion` | `{"n": 3, "k": 2}` |
| `/api/permutacion-con-repeticion` | `{"n": 10, "k": 4}` |
| `/api/permutacion-elementos-repetidos` | `{"palabra": "CASA"}` o `{"cantidades": [2, 1, 1]}` |
| `/api/combinacion-sin-repeticion` | `{"n": 10, "k": 2}` |
| `/api/combinacion-con-repeticion` | `{"n": 4, "k": 3}` |
| `/api/ruta-mas-corta` | `{"aristas": [...], "tipo": "mixto", "origen": "1", "destino": "5"}` |
| `/api/ruta-mas-larga` | igual que la anterior |

Cada arista tiene esta forma: `{"origen": "A", "destino": "B", "peso": 5, "dobleSentido": true}`.
`tipo` puede ser `"no-dirigido"`, `"dirigido"` o `"mixto"`; `dobleSentido` solo se usa cuando el tipo es `"mixto"`.

**Respuesta de conteo:**

```json
{
  "ok": true,
  "formula": "Cₖⁿ = n! / ((n − k)! · k!)",
  "simbolo": "C₂¹⁰",
  "resultado": "45",
  "pasos": [
    { "explicacion": "Expandimos 10! hasta llegar a 8!", "expresion": "10! = 10 × 9 × 8!" },
    …
  ]
}
```

El resultado viaja como **texto** y no como número, porque puede tener cientos de dígitos.

**Respuesta de grafos:** incluye `ruta` (lista de nodos), `pesos` (peso de cada arista usada), `total`, `detalle` (`"5 + 2 + 3 = 10"`) y `pasos`. Además, la ruta más corta agrega `tabla` (la tabla final de Dijkstra) y la ruta más larga agrega `caminos` (los 50 mayores) y `totalCaminos`.

---

## 3. Permutaciones y combinaciones

La notación y el estilo del desarrollo son los mismos de la Clase 1: **Pₖⁿ** y **Cₖⁿ**, con *n* el total de elementos y *k* cuántos se toman.

| Operación | ¿Importa el orden? | ¿Se repiten? | Fórmula |
|---|---|---|---|
| Permutación sin repetición | Sí | No | Pₖⁿ = n! / (n − k)! |
| Permutación con repetición | Sí | Sí | PRₖⁿ = nᵏ |
| Permutación con elementos repetidos | Sí | Hay elementos iguales | P = n! / (n₁! · n₂! · … · nₘ!) |
| Combinación sin repetición | No | No | Cₖⁿ = n! / ((n − k)! · k!) |
| Combinación con repetición | No | Sí | CRₖⁿ = (n + k − 1)! / (k! · (n − 1)!) |

### ¿De dónde sale cada fórmula?

- **Pₖⁿ.** Para el primer lugar hay *n* opciones, para el segundo *n − 1*, y así sucesivamente, *k* veces: n × (n − 1) × … × (n − k + 1). Eso es exactamente n! / (n − k)! (principio multiplicativo).
- **nᵏ.** Cada una de las *k* posiciones puede tomar cualquiera de las *n* opciones: n × n × … × n.
- **n! / (n₁!·n₂!·…).** Si todos los elementos fueran distintos habría n! órdenes. Pero intercambiar entre sí los elementos iguales no produce un orden nuevo, así que se divide entre las n₁!, n₂!, … formas de reordenar cada grupo. Por ejemplo, CASA: 4! / 2! = 12.
- **Cₖⁿ.** Son las permutaciones Pₖⁿ divididas entre las k! formas de ordenar los elementos elegidos, porque en una combinación el orden no cuenta.
- **CRₖⁿ.** Es el método de "estrellas y barras": elegir *k* elementos de *n* tipos equivale a acomodar *k* estrellas y *n − 1* separadores en una fila, lo que da C(n + k − 1, k).

### Cómo se genera el desarrollo (como en clase)

La función `resolverCancelando(total, a, b)` resuelve **total! / (a! · b!)** igual que en el pizarrón:

```
C₂¹⁰ = 10! / (8! · 2!)
10! = 10 × 9 × 8!               ← se expande hasta el factorial MAYOR del denominador
C₂¹⁰ = (10 × 9) / (2 × 1)       ← se cancela 8! arriba y abajo
C₂¹⁰ = 90 / 2                   ← se multiplica
C₂¹⁰ = 45                       ← se divide
```

Cancelar primero el factorial mayor mantiene los números pequeños y muestra el mismo razonamiento de la Clase 1. Las combinaciones con repetición también usan esta función, porque (n + k − 1)! / (k! · (n − 1)!) tiene la misma forma.

### Números grandes (`NumeroGrande`)

`unsigned long long`, el entero más grande de C++, llega hasta 20!. Para que 50!, C(100, 50) o 1000! salgan **exactos**, `NumeroGrande` guarda cada dígito en un `vector` y hace dos operaciones:

- **Multiplicar por un entero pequeño**, dígito por dígito y llevando el acarreo, como la multiplicación de primaria.
- **Dividir entre un entero pequeño** con la división larga de primaria.

Con eso alcanza para todo el proyecto:

- **n!:** se multiplica 1 × 2 × 3 × … × n.
- **Cₖⁿ:** el producto de *k* números seguidos siempre es múltiplo de k!. Por eso, al dividir entre 2, luego entre 3, …, hasta k, **cada división es exacta** y nunca se pierden decimales.

---

## 4. Grafos

### Cómo se escribe el grafo

Primero se elige el **tipo de grafo** (en C++ es el `enum TipoGrafo` de `grafos.h`):

| Tipo | Qué significa | Cómo se guarda cada arista A–B |
|---|---|---|
| **No dirigido** | Todas las aristas se recorren en ambos sentidos (A — B). | Dos conexiones: A→B y B→A. |
| **Dirigido** | Todas las aristas van solo de origen a destino (A → B). | Una conexión: A→B. |
| **Mixto** | Cada arista elige su sentido: → (una vía) o ↔ (ida y vuelta). | Una o dos conexiones, según la arista. |

La regla completa está en una sola función pequeña de `grafos.cpp`:

```cpp
static bool esDobleSentido(const Arista &arista, TipoGrafo tipo) {
    if (tipo == NO_DIRIGIDO) return true;    // no dirigido: todas van y vienen
    if (tipo == DIRIGIDO) return false;      // dirigido: todas van en un solo sentido
    return arista.dobleSentido;              // mixto: lo decide cada arista
}
```

Dijkstra y la búsqueda de la ruta más larga **no cambian** según el tipo: solo cambia qué conexiones se guardan al construir el grafo. El primer paso del desarrollo dice qué tipo de grafo se usó.

El ejemplo de las casas de la Clase 4 es un **grafo mixto**: 1→2, 2→3 y 3→5 son de un solo sentido y las demás aristas son de doble sentido.

Internamente, el grafo es una **lista de adyacencia**: para cada nodo se guarda la lista de nodos a los que se puede ir y cuánto cuesta cada uno.

### Ruta más corta: algoritmo de Dijkstra

```
distancia[origen] = 0;  todas las demás = ∞
repetir:
    tomar el nodo NO visitado con la menor distancia   → esa distancia ya es definitiva
    marcarlo como visitado
    para cada vecino no visitado:
        si distancia[actual] + peso < distancia[vecino]:
            distancia[vecino] = distancia[actual] + peso
            anterior[vecino] = actual
al final: la ruta se reconstruye desde el destino siguiendo "anterior"
```

- **¿Por qué funciona?** Todos los pesos son ≥ 0. Si un nodo es el no visitado más cercano, cualquier otro camino hacia él tendría que pasar por un nodo más lejano, así que no puede ser más corto. Por eso la app **rechaza los pesos negativos**.
- Se usa la versión que busca el mínimo recorriendo la lista, sin cola de prioridad, porque es la más fácil de seguir a mano. Con 15 nodos es instantánea.
- Cada "vuelta" del algoritmo se muestra como un paso, por ejemplo `d(5): 11 → 7 + 3 = 10`. Al final se muestra la tabla completa con distancia y nodo anterior.

### Ruta más larga: DFS con retroceso (backtracking)

La ruta más larga debe ser un **camino simple** (sin repetir nodos). Si se permitiera repetir, un ciclo se podría recorrer infinitas veces y la ruta nunca terminaría.

Este problema es **NP-difícil**: no se conoce un algoritmo rápido como Dijkstra para grafos en general. Por eso la forma **exacta** es **probar todos los caminos simples** y quedarse con el mayor:

```
explorar(nodo):
    si nodo == destino: guardar el camino actual y su peso; regresar
    para cada vecino no visitado:
        avanzar: marcarlo, agregarlo al camino, sumar el peso
        explorar(vecino)
        retroceder: desmarcarlo, quitarlo del camino, restar el peso   ← backtracking
```

- Con los límites de la app (15 nodos) la búsqueda es inmediata en grafos como los de clase.
- Hay un freno de seguridad de 5,000,000 de pasos. Si un grafo muy denso lo supera, la app **avisa** en lugar de dar una respuesta que podría no ser la mayor.
- Si hay empate, se indica cuántos caminos tienen el mismo total.

### Ejemplo de clase (casas 1–7, de 1 a 5)

| Camino | Suma | Total |
|---|---|---|
| 1 → 2 → 3 → 5 | 5 + 6 + 4 | **15** ← más larga |
| 1 → 3 → 5 | 7 + 4 | 11 |
| 1 → 2 → 4 → 5 | 5 + 2 + 3 | **10** ← más corta (coincide con la diapositiva) |

---

## 5. Validaciones

Todas se hacen en C++, así que el resultado es correcto aunque alguien envíe datos sin usar la página.

| Dónde | Qué se valida |
|---|---|
| `main.cpp` | Que el cuerpo sea JSON y que *n*, *k* y los pesos sean **enteros**. Se rechazan decimales, texto y campos vacíos. |
| `combinatoria.cpp` | Que 0 ≤ n, k ≤ 1000; que k ≤ n cuando no hay repetición; que n ≥ 1 en nᵏ y CR; y que las cantidades sean ≥ 1. |
| `grafos.cpp` | Que cada arista tenga origen y destino, que los pesos vayan de 0 a 1,000,000, que haya ≤ 15 nodos y ≤ 60 aristas, que el origen y el destino existan y sean distintos, y que exista una ruta. |

---

## 6. Pruebas

`make test` compila `pruebas/pruebas.cpp`, que llama directamente a las funciones de cálculo y compara con resultados conocidos:

- **Ejemplos de clase:** P₂³ = 6, C₂⁴ = 6, C₂¹⁰ = 45, y en las casas la ruta más corta 1-2-4-5 = 10 y la más larga 1-2-3-5 = 15.
- **Valores grandes:** 25!, C(100, 50) y MISSISSIPPI = 34,650.
- **Los tres tipos de grafo** con las mismas aristas de la Clase 4: por ejemplo, la ruta más corta 7→1 es 7‑5‑4‑2‑3‑1 = 23 si el grafo es mixto, 7‑5‑4‑2‑1 = 15 si es no dirigido, y no existe si es dirigido.
- **Datos inválidos** que se deben rechazar: k > n, pesos negativos, nodos inexistentes y rutas imposibles por el sentido de las aristas.

---

## 7. Guion sugerido para explicar el código en clase

1. **`web/app.js` → `llamarServidor`:** la página solo envía datos.
2. **`src/main.cpp` → `registrarOperacionNK`:** el servidor recibe los datos y llama a la función.
3. **`src/combinatoria.cpp` → `combinacionSinRepeticion` y `resolverCancelando`:** la fórmula y el desarrollo, igual que en el pizarrón.
4. **`src/numero_grande.cpp`:** por qué los factoriales necesitan números grandes.
5. **`src/grafos.cpp` → `rutaMasCorta`:** Dijkstra en unas 30 líneas, con el ejemplo de las casas en pantalla.
6. **`src/grafos.cpp` → `explorar`:** el backtracking y por qué la ruta más larga no se resuelve con Dijkstra.
7. **`make test`:** demostrar que los ejemplos de clase dan exactamente lo mismo.
