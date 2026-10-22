#!/bin/bash
# =====================================================================
#  Calculadora de Matemática Discreta — iniciador para macOS
# =====================================================================
#  Doble clic en este archivo para:
#    1. Revisar que estén las herramientas de compilación de Apple.
#    2. Compilar el programa (solo la primera vez o si hubo cambios).
#    3. Buscar un puerto libre y levantar el servidor.
#    4. Abrir la calculadora en el navegador.
#  Para cerrar la calculadora, cierra esta ventana de Terminal o presiona Ctrl + C.

# Trabajar siempre desde la carpeta del proyecto (la ruta puede tener espacios).
cd "$(dirname "$0")" || exit 1

pausar() {
    echo
    read -n 1 -s -r -p "Presiona cualquier tecla para cerrar..."
    echo
}

echo
echo "  Calculadora de Matemática Discreta"
echo "  ----------------------------------"

# 1. Herramientas de compilación (clang++ y make vienen en las "Command Line Tools" de Apple)
if ! xcode-select -p >/dev/null 2>&1 || ! xcrun --find clang++ >/dev/null 2>&1; then
    echo
    echo "  Falta instalar las herramientas de compilación de Apple (Command Line Tools)."
    echo "  Se abrirá una ventana para instalarlas: presiona \"Instalar\" y espera a que termine."
    echo "  Después vuelve a abrir este archivo."
    xcode-select --install >/dev/null 2>&1
    pausar
    exit 1
fi

# 2. Compilar. make solo recompila lo que cambió; la primera vez tarda unos segundos.
if [ ! -x build/calculadora ]; then
    echo "  Compilando por primera vez (tarda unos segundos)..."
fi
if ! make -s >/tmp/calculadora-compilacion.log 2>&1; then
    echo
    echo "  No se pudo compilar. Detalle del error:"
    echo
    cat /tmp/calculadora-compilacion.log
    pausar
    exit 1
fi

# 3. Buscar un puerto libre a partir del 8080.
#    Si la calculadora ya está abierta en ese puerto, solo se abre el navegador.
PUERTO=8080
while lsof -nP -iTCP:"$PUERTO" -sTCP:LISTEN >/dev/null 2>&1; do
    if lsof -nP -iTCP:"$PUERTO" -sTCP:LISTEN -Fc 2>/dev/null | grep -qx "ccalculadora"; then
        echo "  La calculadora ya estaba abierta. Abriendo el navegador..."
        open "http://localhost:$PUERTO"
        exit 0
    fi
    PUERTO=$((PUERTO + 1))
    if [ "$PUERTO" -gt 8099 ]; then
        echo "  No se encontró un puerto libre entre 8080 y 8099."
        pausar
        exit 1
    fi
done

# 4. Abrir el navegador en cuanto el servidor responda (se revisa cada 0.2 s, hasta 10 s).
(
    for _ in $(seq 1 50); do
        if curl -s -o /dev/null "http://localhost:$PUERTO/"; then
            open "http://localhost:$PUERTO"
            exit 0
        fi
        sleep 0.2
    done
) &

# El servidor queda corriendo en esta ventana hasta que se cierre.
./build/calculadora "$PUERTO"
