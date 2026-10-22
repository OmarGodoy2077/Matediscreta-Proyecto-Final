# Calculadora de Matemática Discreta

Proyecto final del curso de **Matemática Discreta**. Es una aplicación escrita en **C++** que resuelve problemas de conteo y de grafos y muestra el **desarrollo paso a paso**, con una interfaz web sencilla que se abre en el navegador.

| Categoría | Operaciones |
|---|---|
| **Permutaciones** | Sin repetición · Con repetición (nᵏ y elementos repetidos / anagramas) |
| **Combinaciones** | Sin repetición · Con repetición |
| **Grafos** | Ruta más corta (Dijkstra) · Ruta más larga (DFS con retroceso) |

## Inicio rápido en macOS

1. **Solo la primera vez**, instala las herramientas de compilación de Apple desde la Terminal:
   ```bash
   xcode-select --install
   ```
2. Haz **doble clic en `Abrir Calculadora.command`**. El archivo compila el programa, levanta el servidor y abre la calculadora en el navegador.
3. Para cerrarla, cierra la ventana de Terminal que se abrió.

La guía completa, con los avisos de seguridad de macOS y la solución de problemas, está en **[docs/instalacion-mac.md](docs/instalacion-mac.md)**.

### Desde la Terminal

No hay que instalar librerías: las dos que se usan ya vienen en la carpeta `lib/`.

```bash
make run               # compila y levanta el servidor → http://localhost:8080
make test              # corre las pruebas automáticas
make run PUERTO=3000   # usa otro puerto
make clean             # borra lo compilado
```

Para detener el servidor presiona `Ctrl + C`.

> **Linux y Windows:** se necesita `g++` y `make` (en Windows, con MSYS2). Las instrucciones están en [docs/estructura-y-stack.md](docs/estructura-y-stack.md#compilar-en-cada-sistema).

## Características

- **Todos los cálculos se hacen en C++.** La página web solo manda los datos y muestra la respuesta.
- **Desarrollo paso a paso** con la misma notación de clase (Pₖⁿ, Cₖⁿ): se expande el factorial, se cancela y se simplifica.
- **Resultados exactos** aunque sean enormes (por ejemplo 1000!), gracias a una clase propia de números grandes.
- **Grafos dirigidos, no dirigidos y mixtos:** se elige el tipo de grafo; en un grafo mixto cada arista puede ser de un solo sentido (→) o de doble sentido (↔). El grafo se dibuja al momento.
- Validación con mensajes claros, modo claro/oscuro y diseño que funciona en celular.
- Funciona **sin internet** porque no depende de ningún CDN.

## Documentación

| Documento | Para quién |
|---|---|
| [Instalación en macOS](docs/instalacion-mac.md) | Quien la va a abrir por primera vez en una Mac |
| [Manual de usuario](docs/manual-usuario.md) | Quien va a usar la calculadora |
| [Funcionamiento](docs/funcionamiento.md) | Quien quiere entender cómo resuelve cada operación |
| [Estructura y stack](docs/estructura-y-stack.md) | Quien va a leer, compilar o modificar el código |
| [Estado del proyecto](docs/estado-proyecto.md) | Avance, pruebas y pendientes |
