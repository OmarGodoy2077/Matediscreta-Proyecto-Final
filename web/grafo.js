// =====================================================================
//  Dibujo del grafo en SVG
// =====================================================================
//  Este archivo SOLO dibuja. No calcula rutas: eso lo hace el servidor en C++.
//
//  Los nodos se acomodan en un círculo. Las aristas de un solo sentido (→)
//  llevan flecha; las de doble sentido (↔) son una línea simple.
//  El tipo de grafo decide el sentido: "no-dirigido" (todas sin flecha),
//  "dirigido" (todas con flecha) o "mixto" (cada arista elige).
//  Si hay una ruta resaltada, sus nodos y aristas se pintan de otro color.

const SVG_NS = 'http://www.w3.org/2000/svg';
const ANCHO = 560;
const ALTO = 420;
const RADIO_NODO = 21;

function crearSvg(etiqueta, atributos) {
    const elemento = document.createElementNS(SVG_NS, etiqueta);
    for (const [nombre, valor] of Object.entries(atributos)) {
        elemento.setAttribute(nombre, valor);
    }
    return elemento;
}

// ¿La arista se puede recorrer en ambos sentidos? (misma regla que grafos.cpp)
function esDobleSentido(arista, tipo) {
    if (tipo === 'no-dirigido') return true;
    if (tipo === 'dirigido') return false;
    return arista.dobleSentido;   // mixto: lo decide cada arista
}

// Lista de nodos sin repetir, en el orden en que aparecen en las aristas.
function obtenerNodos(aristas) {
    const nodos = [];
    for (const a of aristas) {
        for (const nombre of [a.origen, a.destino]) {
            if (nombre && !nodos.includes(nombre)) nodos.push(nombre);
        }
    }
    return nodos;
}

// Posición (x, y) de cada nodo alrededor de un círculo, empezando arriba.
function calcularPosiciones(nodos) {
    const posiciones = {};
    const centroX = ANCHO / 2;
    const centroY = ALTO / 2;
    const radio = nodos.length <= 2 ? 150 : Math.min(170, 60 + nodos.length * 16);

    nodos.forEach((nombre, i) => {
        if (nodos.length === 1) {
            posiciones[nombre] = { x: centroX, y: centroY };
            return;
        }
        const angulo = -Math.PI / 2 + (2 * Math.PI * i) / nodos.length;
        posiciones[nombre] = {
            x: centroX + radio * Math.cos(angulo),
            y: centroY + radio * Math.sin(angulo),
        };
    });
    return posiciones;
}

// Decide qué aristas forman parte de la ruta, comparando nodos y peso.
function aristasDeLaRuta(aristas, tipo, ruta) {
    const usadas = new Set();
    if (!ruta) return usadas;

    for (let i = 0; i + 1 < ruta.nodos.length; i++) {
        const desde = ruta.nodos[i];
        const hasta = ruta.nodos[i + 1];
        const peso = ruta.pesos[i];
        const indice = aristas.findIndex((a, j) =>
            !usadas.has(j) &&
            Number(a.peso) === peso &&
            ((a.origen === desde && a.destino === hasta) ||
             (esDobleSentido(a, tipo) && a.origen === hasta && a.destino === desde)));
        if (indice !== -1) usadas.add(indice);
    }
    return usadas;
}

// Dibuja la flecha (marcador) que se usa en las aristas dirigidas.
function crearDefiniciones() {
    const defs = crearSvg('defs', {});
    for (const [id, clase] of [['flecha', 'g-flecha'], ['flecha-ruta', 'g-flecha-ruta']]) {
        const marcador = crearSvg('marker', {
            id, viewBox: '0 0 10 10', refX: 9, refY: 5,
            markerWidth: 7, markerHeight: 7, orient: 'auto-start-reverse',
        });
        marcador.appendChild(crearSvg('path', { d: 'M0,0 L10,5 L0,10 Z', class: clase }));
        defs.appendChild(marcador);
    }
    return defs;
}

// Mueve el punto "desde" una distancia hacia el punto "hacia".
function acercar(desde, hacia, distancia) {
    const dx = hacia.x - desde.x;
    const dy = hacia.y - desde.y;
    const largo = Math.hypot(dx, dy) || 1;
    return { x: desde.x + (dx / largo) * distancia, y: desde.y + (dy / largo) * distancia };
}

function dibujarPeso(svg, x, y, peso, enRuta) {
    const texto = String(peso);
    const ancho = 12 + texto.length * 8;
    const clase = enRuta ? ' en-ruta' : '';
    svg.appendChild(crearSvg('rect', {
        x: x - ancho / 2, y: y - 11, width: ancho, height: 22, rx: 6, class: 'g-peso-fondo' + clase,
    }));
    const etiqueta = crearSvg('text', { x, y: y + 4.5, 'text-anchor': 'middle', class: 'g-peso' });
    etiqueta.textContent = texto;
    svg.appendChild(etiqueta);
}

function dibujarGrafo(svg, aristas, tipo, ruta) {
    svg.replaceChildren();
    const validas = aristas.filter(a => a.origen && a.destino);
    const nodos = obtenerNodos(validas);

    if (nodos.length === 0) {
        const aviso = crearSvg('text', { x: ANCHO / 2, y: ALTO / 2, 'text-anchor': 'middle', class: 'g-vacio' });
        aviso.textContent = 'Agrega aristas para ver el grafo aquí';
        svg.appendChild(aviso);
        return;
    }

    svg.appendChild(crearDefiniciones());
    const pos = calcularPosiciones(nodos);
    const enRuta = aristasDeLaRuta(validas, tipo, ruta);
    const nodosEnRuta = new Set(ruta ? ruta.nodos : []);

    // Contamos cuántas aristas unen el mismo par de nodos, para curvarlas y que no se encimen.
    const clavePar = a => [a.origen, a.destino].sort().join('\u0000');
    const totalPorPar = {};
    for (const a of validas) totalPorPar[clavePar(a)] = (totalPorPar[clavePar(a)] || 0) + 1;
    const vistasPorPar = {};

    // Las aristas de la ruta se dibujan al final para que queden encima.
    const orden = validas.map((a, i) => i).sort((i, j) => enRuta.has(i) - enRuta.has(j));

    for (const i of orden) {
        const a = validas[i];
        const resaltada = enRuta.has(i);
        const clase = 'g-arista' + (resaltada ? ' en-ruta' : '');
        const flecha = esDobleSentido(a, tipo) ? null : `url(#${resaltada ? 'flecha-ruta' : 'flecha'})`;
        const p1 = pos[a.origen];
        const p2 = pos[a.destino];

        // Lazo (arista de un nodo a sí mismo): un pequeño arco encima del nodo.
        if (a.origen === a.destino) {
            const y = p1.y - RADIO_NODO;
            const lazo = crearSvg('path', {
                d: `M${p1.x - 9},${y + 3} C${p1.x - 34},${y - 46} ${p1.x + 34},${y - 46} ${p1.x + 9},${y + 3}`,
                class: clase,
            });
            if (flecha) lazo.setAttribute('marker-end', flecha);
            svg.appendChild(lazo);
            dibujarPeso(svg, p1.x, y - 38, a.peso, resaltada);
            continue;
        }

        // Punto de control de la curva: en medio de los nodos, desplazado si hay aristas paralelas.
        const clave = clavePar(a);
        const numero = vistasPorPar[clave] = (vistasPorPar[clave] || 0) + 1;
        const desplazamiento = (numero - 1 - (totalPorPar[clave] - 1) / 2) * 34;
        const [primero] = [a.origen, a.destino].sort();
        const sentido = a.origen === primero ? 1 : -1;   // mismo lado sin importar la dirección
        const largo = Math.hypot(p2.x - p1.x, p2.y - p1.y) || 1;
        const control = {
            x: (p1.x + p2.x) / 2 - ((p2.y - p1.y) / largo) * desplazamiento * sentido,
            y: (p1.y + p2.y) / 2 + ((p2.x - p1.x) / largo) * desplazamiento * sentido,
        };

        const inicio = acercar(p1, control, RADIO_NODO + 1);
        const fin = acercar(p2, control, RADIO_NODO + 3);
        const linea = crearSvg('path', {
            d: `M${inicio.x},${inicio.y} Q${control.x},${control.y} ${fin.x},${fin.y}`,
            class: clase,
        });
        if (flecha) linea.setAttribute('marker-end', flecha);
        svg.appendChild(linea);

        // El peso va en el punto medio de la curva.
        const medio = {
            x: 0.25 * inicio.x + 0.5 * control.x + 0.25 * fin.x,
            y: 0.25 * inicio.y + 0.5 * control.y + 0.25 * fin.y,
        };
        dibujarPeso(svg, medio.x, medio.y, a.peso, resaltada);
    }

    // Nodos encima de las aristas.
    for (const nombre of nodos) {
        const { x, y } = pos[nombre];
        const clase = nodosEnRuta.has(nombre) ? ' en-ruta' : '';
        svg.appendChild(crearSvg('circle', { cx: x, cy: y, r: RADIO_NODO, class: 'g-nodo' + clase }));
        const etiqueta = crearSvg('text', { x, y: y + 5, 'text-anchor': 'middle', class: 'g-etiqueta' + clase });
        etiqueta.textContent = nombre.length > 4 ? nombre.slice(0, 4) + '…' : nombre;
        svg.appendChild(etiqueta);
    }
}
