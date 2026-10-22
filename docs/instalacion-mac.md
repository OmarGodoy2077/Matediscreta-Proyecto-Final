# Instalación y uso en macOS

Guía paso a paso para abrir la calculadora en una Mac. Funciona en Mac con chip Apple (M1, M2, M3…) y con procesador Intel.

**Resumen:** instala una sola vez las herramientas de Apple y después abre **`Abrir Calculadora.command`** con doble clic.

---

## Paso 1. Instalar las herramientas de compilación (solo la primera vez)

La calculadora está escrita en C++, así que la Mac necesita el compilador de Apple. Viene en un paquete gratuito llamado **Command Line Tools** (unos 700 MB). No hace falta instalar Xcode completo.

1. Abre la app **Terminal** (Aplicaciones → Utilidades → Terminal, o busca "Terminal" con `Cmd + Espacio`).
2. Escribe este comando y presiona Enter:

   ```bash
   xcode-select --install
   ```

3. Aparece una ventana. Presiona **Instalar**, acepta la licencia y espera a que termine (5 a 15 minutos, según la conexión).

> Si aparece el mensaje *"command line tools are already installed"*, ya las tienes y puedes pasar al paso 2.

**Para comprobar que quedaron instaladas**, ejecuta:

```bash
clang++ --version && make --version
```

Deben aparecer las versiones de ambos programas, sin errores.

> **Atajo:** si abres `Abrir Calculadora.command` sin tener las herramientas, el propio archivo lo detecta y abre la ventana de instalación.

---

## Paso 2. Abrir la calculadora

### Opción A — Con doble clic (recomendada)

1. Abre la carpeta del proyecto en Finder.
2. Haz **doble clic** en **`Abrir Calculadora.command`**.
3. Se abre una ventana de Terminal y, pocos segundos después, la calculadora aparece en el navegador.

Qué hace el archivo por ti:

- Revisa que estén las herramientas de Apple.
- Compila el programa. La primera vez tarda unos segundos; las siguientes veces es inmediato.
- Busca un puerto libre, empezando por el 8080.
- Abre la calculadora en tu navegador predeterminado.
- Si la calculadora ya estaba abierta, solo vuelve a abrir el navegador.

> **Importante:** la ventana de Terminal es el "motor" de la calculadora. **No la cierres mientras la estés usando.** Puedes minimizarla.

### Opción B — Desde la Terminal

1. Abre Terminal y entra a la carpeta del proyecto. Como el nombre tiene espacios, lo más fácil es escribir `cd ` (con un espacio al final), **arrastrar la carpeta del proyecto** desde Finder a la Terminal y presionar Enter.

   ```bash
   cd "/ruta/a/Matediscreta Proyecto Final"
   ```

2. Compila y levanta el servidor:

   ```bash
   make run
   ```

3. Abre **http://localhost:8080** en Safari, Chrome o Firefox.

---

## Paso 3. Cerrar la calculadora

Tienes dos formas:

- Cierra la ventana de Terminal. Si macOS pregunta si quieres terminar el proceso, responde **Terminar**.
- O, dentro de la Terminal, presiona **`Ctrl + C`**.

Cerrar la pestaña del navegador **no** detiene la calculadora: sigue corriendo hasta que cierres la Terminal.

---

## Comandos útiles

Todos se ejecutan dentro de la carpeta del proyecto:

| Comando | Qué hace |
|---|---|
| `make run` | Compila (si hace falta) y levanta la calculadora en el puerto 8080. |
| `make run PUERTO=3000` | Usa otro puerto. |
| `make test` | Corre las pruebas automáticas (deben salir 30 aprobadas). |
| `make` | Solo compila. |
| `make clean` | Borra lo compilado; lo siguiente que ejecutes compila desde cero. |

---

## Solución de problemas

### Al hacer doble clic: *"no se puede abrir porque Apple no puede verificar que esté libre de software malicioso"*

Esto pasa cuando el proyecto se descargó de internet (GitHub, correo, Drive, AirDrop). macOS bloquea los archivos descargados que no están firmados. Hay dos soluciones:

**Opción 1 — Desde Ajustes:**

1. En el aviso, presiona **Listo** (no lo mandes a la papelera).
2. Abre **Ajustes del Sistema → Privacidad y seguridad**.
3. Baja hasta el mensaje sobre *"Abrir Calculadora.command"* y presiona **Abrir igualmente**.
4. Confirma con tu contraseña. Solo hay que hacerlo la primera vez.

**Opción 2 — Desde Terminal** (quita el bloqueo a todo el proyecto de una vez):

```bash
xattr -dr com.apple.quarantine "/ruta/a/Matediscreta Proyecto Final"
```

### *"no tienes los permisos adecuados"* o el archivo se abre como texto

El archivo perdió el permiso de ejecución, algo que puede pasar al comprimirlo o copiarlo. Ejecuta dentro de la carpeta del proyecto:

```bash
chmod +x "Abrir Calculadora.command"
```

### *"xcrun: error: invalid active developer path"*

Suele pasar después de actualizar macOS, porque la actualización desactiva las herramientas de Apple. Reinstálalas con:

```bash
xcode-select --install
```

### *"zsh: command not found: make"* o *"clang++: command not found"*

Faltan las herramientas de Apple. Vuelve al [Paso 1](#paso-1-instalar-las-herramientas-de-compilación-solo-la-primera-vez).

### *"No se pudo usar el puerto 8080"* (con `make run`)

Otro programa está usando ese puerto. Tienes dos opciones:

- Usa otro puerto: `make run PUERTO=3000` y abre `http://localhost:3000`.
- O usa `Abrir Calculadora.command`, que busca un puerto libre por su cuenta.

Para ver qué programa ocupa el puerto:

```bash
lsof -nP -iTCP:8080 -sTCP:LISTEN
```

### El navegador dice *"No se puede conectar"* o la página muestra *"No se pudo conectar con el servidor"*

La calculadora no está corriendo: seguramente se cerró la ventana de Terminal. Vuelve a abrir `Abrir Calculadora.command`.

### *"No se encontró la carpeta web"*

Al proyecto le falta la carpeta `web/`. Vuelve a copiar el proyecto completo.

### Hice cambios en el código y no se ven

- **Cambios en C++ (`src/`):** hay que recompilar. Cierra la Terminal y vuelve a abrir `Abrir Calculadora.command`; `make` recompila solo lo que cambió.
- **Cambios en la página (`web/`):** basta con recargar el navegador con `Cmd + Shift + R`.

---

## Información técnica

| Elemento | Detalle |
|---|---|
| Compilador | `clang++` de Apple, estándar C++17 (el `Makefile` lo usa por medio de `c++`/`g++`) |
| Make | GNU Make 3.81 (el que incluye macOS) |
| Probado en | macOS 27 · Apple Silicon (arm64) |
| Dirección | La calculadora solo escucha en `127.0.0.1` (esta Mac). No se puede abrir desde otra computadora de la red. |
| Internet | No hace falta: todo funciona sin conexión. |
