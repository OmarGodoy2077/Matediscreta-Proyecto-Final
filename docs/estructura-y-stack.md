# Estructura del proyecto y stack

## Stack tecnológico

| Capa | Tecnología | Por qué |
|---|---|---|
| Lógica (cálculos) | **C++17**, solo con la librería estándar | Es el lenguaje principal que pide el curso. Toda la matemática vive aquí. |
| Servidor web | [cpp-httplib](https://github.com/yhirose/cpp-httplib) **v0.58.0** | Es un solo archivo `.h` y no hay que instalar nada. Sirve la página y recibe los cálculos. |
| Formato de datos | [nlohmann/json](https://github.com/nlohmann/json) **v3.12.0** | Es un solo archivo `.hpp`. Lee y escribe JSON de forma legible. |
| Interfaz | **HTML + CSS + JavaScript**, sin frameworks ni CDN | Se entiende sin herramientas extra y funciona sin internet. |
| Dibujo del grafo | **SVG** generado con JavaScript | Es nativo del navegador y no necesita librerías. |
| Compilación | **Make** + `g++`/`clang++` | Un comando para compilar, correr y probar. |

Las dos librerías externas ya están guardadas en `lib/` (las dos tienen licencia MIT), así que **no hay que descargar dependencias**. Si en el futuro hiciera falta alguna dependencia de JavaScript, se instalará con **pnpm**.

---

## Estructura de carpetas

```
Matediscreta Proyecto Final/
├── README.md                 Presentación y arranque rápido
├── Abrir Calculadora.command Iniciador para macOS (doble clic): compila, levanta y abre el navegador
├── Makefile                  Compilar (make), correr (make run), probar (make test)
├── .gitignore
│
├── src/                      Código C++ del proyecto
│   ├── main.cpp              Servidor web: recibe datos, llama al cálculo, responde JSON
│   ├── combinatoria.h/.cpp   Permutaciones y combinaciones, con sus pasos
│   ├── grafos.h/.cpp         Ruta más corta (Dijkstra) y más larga (DFS)
│   ├── numero_grande.h/.cpp  Enteros sin límite de dígitos (para factoriales grandes)
│   └── paso.h                Estructura de un paso del desarrollo
│
├── lib/                      Librerías externas (no se modifican)
│   ├── httplib.h
│   └── json.hpp
│
├── web/                      Interfaz que ve el usuario
│   ├── index.html            Estructura: menú, formularios y zonas de resultado
│   ├── estilos.css           Diseño, colores y modo oscuro
│   ├── app.js                Navegación, formularios y llamadas al servidor
│   └── grafo.js              Dibujo del grafo en SVG
│
├── pruebas/
│   └── pruebas.cpp           Pruebas automáticas con los ejemplos de clase
│
├── docs/                     Documentación
│   ├── instalacion-mac.md
│   ├── manual-usuario.md
│   ├── funcionamiento.md
│   ├── estructura-y-stack.md
│   ├── estado-proyecto.md
│   ├── guion-exposicion.md   (personal: explicación del código en la exposición; solo local)
│   └── demo-sistema.md       (personal: demostración de la página en la exposición; solo local)
│
├── build/                    (se genera al compilar; no se sube a git)
└── Docs clases /             (material de clase; solo local, está en .gitignore)
```

### Responsabilidad de cada archivo C++

El código está separado para que cada parte tenga **una sola responsabilidad**:

- **`combinatoria.cpp` y `grafos.cpp`** no saben nada de internet ni de JSON. Reciben números o aristas y devuelven una estructura (`ResultadoConteo` o `ResultadoGrafo`) con el resultado, los pasos o un mensaje de error. Por eso se pueden probar solos con `pruebas.cpp`.
- **`main.cpp`** es solo el "mesero": convierte el JSON que llega en datos de C++, llama a la función correcta y convierte la respuesta en JSON.
- **`numero_grande.cpp`** guarda un número dígito por dígito y hace multiplicación y división "de primaria". Existe porque 21! ya no cabe en un `unsigned long long`.

### Convenciones del código

- Nombres en español y descriptivos: `permutacionSinRepeticion`, `rutaMasCorta`, `resolverCancelando`.
- Funciones cortas, con un comentario que explica el **porqué** de cada bloque.
- Los errores de datos se devuelven con `ok = false` y un mensaje para el usuario, sin excepciones.
- Los límites están en constantes con nombre (`MAXIMO_N`, `MAXIMO_NODOS`…) y no como números sueltos.
- Compila sin advertencias con `-Wall -Wextra`.

---

## Compilar en cada sistema

### macOS

```bash
xcode-select --install     # solo la primera vez (instala clang++ y make)
make run
```

O con doble clic en `Abrir Calculadora.command`. Paso a paso y solución de problemas en [instalacion-mac.md](instalacion-mac.md).

**Cómo funciona el iniciador `.command`:** es un script de bash que macOS abre en Terminal al hacer doble clic. Hace lo siguiente, en orden:

1. Entra a la carpeta del proyecto (funciona aunque la ruta tenga espacios).
2. Revisa con `xcode-select -p` que estén las Command Line Tools; si faltan, lanza el instalador.
3. Ejecuta `make`, que solo recompila lo que cambió.
4. Busca un puerto libre desde el 8080 con `lsof`. Si en ese puerto ya corre la calculadora, solo abre el navegador.
5. Espera a que el servidor responda y lo abre con `open http://localhost:PUERTO`.

### Linux (Ubuntu/Debian)

```bash
sudo apt install g++ make  # solo la primera vez
make run
```

### Windows

1. Instala [MSYS2](https://www.msys2.org/) y, en su terminal **UCRT64**, ejecuta:
   `pacman -S mingw-w64-ucrt-x86_64-gcc make`
2. Desde esa misma terminal, entra a la carpeta del proyecto y ejecuta `make run`.

El `Makefile` detecta Windows y agrega la librería de red `ws2_32`. Se necesita Windows 10 o superior.

### Sin Make (un solo comando)

```bash
mkdir -p build
g++ -std=c++17 -O2 -Ilib -Isrc src/*.cpp -o build/calculadora -pthread      # macOS / Linux
g++ -std=c++17 -O2 -Ilib -Isrc src/*.cpp -o build/calculadora.exe -lws2_32  # Windows
./build/calculadora
```

> El programa busca la página primero en `./web` y, si no la encuentra, en la carpeta `web` junto a `build/`. Por eso funciona aunque se ejecute desde otra carpeta.
