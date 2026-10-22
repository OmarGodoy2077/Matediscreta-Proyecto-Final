# =====================================================================
#  Calculadora de Matemática Discreta
# =====================================================================
#  make          → compila el programa
#  make run      → compila y levanta el servidor (http://localhost:8080)
#  make test     → compila y corre las pruebas
#  make clean    → borra lo compilado
# =====================================================================

CXX      ?= g++
CXXFLAGS  = -std=c++17 -Wall -Wextra -O2 -Ilib -Isrc
PUERTO   ?= 8080

# Windows necesita la librería de red "ws2_32"; macOS y Linux usan hilos (pthread).
ifeq ($(OS),Windows_NT)
    LDFLAGS = -lws2_32
    EXT     = .exe
else
    LDFLAGS = -pthread
    EXT     =
endif

# Código de los cálculos (lo usan tanto el servidor como las pruebas).
LOGICA   = build/numero_grande.o build/combinatoria.o build/grafos.o
APP      = build/calculadora$(EXT)
PRUEBAS  = build/pruebas$(EXT)

all: $(APP)

$(APP): build/main.o $(LOGICA)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(PRUEBAS): build/pruebas.o $(LOGICA)
	$(CXX) $^ -o $@ $(LDFLAGS)

# Cada .cpp se compila por separado; así, si cambias grafos.cpp,
# no hay que volver a compilar main.cpp (que tarda más por las librerías).
build/%.o: src/%.cpp src/*.h
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/pruebas.o: pruebas/pruebas.cpp src/*.h
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(APP)
	./$(APP) $(PUERTO)

test: $(PRUEBAS)
	./$(PRUEBAS)

clean:
	rm -rf build

.PHONY: all run test clean
