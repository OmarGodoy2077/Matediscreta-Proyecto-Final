// =====================================================================
//  Calculadora de Matemática Discreta — lógica de la página
// =====================================================================
//  Este archivo NO hace cálculos matemáticos. Su trabajo es:
//    1. Mostrar la vista que eligió el usuario en el menú.
//    2. Leer los datos del formulario y enviarlos al servidor en C++.
//    3. Mostrar el resultado y los pasos que devuelve el servidor.

// ---------------------------------------------------------------------
// 1. Configuración de cada operación (textos y ejemplos de clase)
// ---------------------------------------------------------------------

const OPERACIONES_CONTEO = {
    'permutacion-sin-repeticion': {
        grupo: 'Permutaciones',
        titulo: 'Permutación sin repetición',
        descripcion: 'El orden sí importa y cada elemento se usa una sola vez.',
        formula: 'Pₖⁿ = n! / (n − k)!',
        api: 'permutacion-sin-repeticion',
        etiquetaN: 'n · total de elementos',
        etiquetaK: 'k · cuántos se ordenan',
        ejemplo: {
            n: 3, k: 2,
            texto: 'Eduardo, Carlos y Sergio compiten por el 1.er y 2.º lugar. ¿De cuántas formas se reparten los premios? (Clase 1)',
        },
    },
    'permutacion-con-repeticion': {
        grupo: 'Permutaciones',
        titulo: 'Permutación con repetición',
        descripcion: 'El orden sí importa y los elementos se pueden repetir.',
        formula: 'PRₖⁿ = nᵏ',
        api: 'permutacion-con-repeticion',
        etiquetaN: 'n · opciones para cada posición',
        etiquetaK: 'k · posiciones a llenar',
        ejemplo: {
            n: 10, k: 4,
            texto: '¿Cuántos PIN de 4 dígitos existen? Cada dígito puede ser cualquiera del 0 al 9.',
        },
        // Segunda variante: n! / (n₁! · n₂! · …)
        elementos: {
            descripcion: 'El orden importa, pero algunos elementos son iguales entre sí (como las letras de una palabra).',
            formula: 'P = n! / (n₁! · n₂! · … · nₘ!)',
            api: 'permutacion-elementos-repetidos',
            ejemplo: {
                palabra: 'CASA',
                texto: '¿Cuántas palabras distintas se forman reordenando las letras de CASA? (la A se repite 2 veces)',
            },
        },
    },
    'combinacion-sin-repeticion': {
        grupo: 'Combinaciones',
        titulo: 'Combinación sin repetición',
        descripcion: 'El orden no importa y cada elemento se usa una sola vez.',
        formula: 'Cₖⁿ = n! / ((n − k)! · k!)',
        api: 'combinacion-sin-repeticion',
        etiquetaN: 'n · total de elementos',
        etiquetaK: 'k · cuántos se eligen',
        ejemplo: {
            n: 10, k: 2,
            texto: 'Torneo de ajedrez con 10 jugadores: cada uno juega una vez contra cada uno de los demás. ¿Cuántos partidos hay? (Clase 1)',
        },
    },
    'combinacion-con-repeticion': {
        grupo: 'Combinaciones',
        titulo: 'Combinación con repetición',
        descripcion: 'El orden no importa y los elementos se pueden repetir.',
        formula: 'CRₖⁿ = (n + k − 1)! / (k! · (n − 1)!)',
        api: 'combinacion-con-repeticion',
        etiquetaN: 'n · tipos de elementos',
        etiquetaK: 'k · cuántos se eligen',
        ejemplo: {
            n: 4, k: 3,
            texto: 'Una heladería tiene 4 sabores. ¿De cuántas formas se puede pedir un cono de 3 bolas si se pueden repetir sabores?',
        },
    },
};

const OPERACIONES_GRAFO = {
    'ruta-mas-corta': {
        titulo: 'Ruta más corta',
        descripcion: 'Encuentra el camino de menor peso total entre dos nodos usando el algoritmo de Dijkstra.',
        api: 'ruta-mas-corta',
        boton: 'Calcular ruta más corta',
    },
    'ruta-mas-larga': {
        titulo: 'Ruta más larga',
        descripcion: 'Encuentra el camino de mayor peso total entre dos nodos, sin pasar dos veces por el mismo nodo.',
        api: 'ruta-mas-larga',
        boton: 'Calcular ruta más larga',
    },
};

// Tipos de grafo. Quien decide el sentido de cada arista es el servidor en C++;
// aquí solo está el texto de ayuda que ve el usuario.
const TIPOS_GRAFO = {
    'no-dirigido': 'Grafo no dirigido: cada arista se puede recorrer en ambos sentidos (A — B).',
    'dirigido': 'Grafo dirigido: cada arista solo se recorre de origen a destino (A → B).',
    'mixto': 'Grafo mixto: cada arista elige su sentido. → solo de origen a destino; ↔ en ambos sentidos.',
};

// Grafo del ejemplo de las casas (Clase 4). Es mixto: tiene calles de un solo sentido y de doble sentido.
const EJEMPLO_GRAFO = {
    tipo: 'mixto',
    aristas: [
        { origen: '1', destino: '2', peso: 5, dobleSentido: false },
        { origen: '1', destino: '3', peso: 7, dobleSentido: true },
        { origen: '2', destino: '3', peso: 6, dobleSentido: false },
        { origen: '2', destino: '4', peso: 2, dobleSentido: true },
        { origen: '3', destino: '5', peso: 4, dobleSentido: false },
        { origen: '4', destino: '5', peso: 3, dobleSentido: true },
        { origen: '5', destino: '6', peso: 2, dobleSentido: true },
        { origen: '5', destino: '7', peso: 5, dobleSentido: true },
    ],
    origen: '1',
    destino: '5',
};

// ---------------------------------------------------------------------
// Ayudas
// ---------------------------------------------------------------------

const $ = selector => document.querySelector(selector);

// Crea un elemento HTML con texto (textContent evita inyectar HTML).
function crear(etiqueta, clase, texto) {
    const elemento = document.createElement(etiqueta);
    if (clase) elemento.className = clase;
    if (texto !== undefined) elemento.textContent = texto;
    return elemento;
}

// "2598960" → "2,598,960"
function formatearNumero(texto) {
    return texto.replace(/\B(?=(\d{3})+(?!\d))/g, ',');
}

// Convierte un texto a número. Si está vacío se manda null
// y el servidor responde con un mensaje claro.
function aNumero(texto) {
    return String(texto).trim() === '' ? null : Number(texto);
}

// Guardar/leer en el navegador. Si el navegador no lo permite, simplemente no se guarda.
function guardarLocal(clave, valor) {
    try { localStorage.setItem(clave, JSON.stringify(valor)); } catch (e) { /* sin almacenamiento */ }
}
function leerLocal(clave) {
    try { return JSON.parse(localStorage.getItem(clave)); } catch (e) { return null; }
}

// ---------------------------------------------------------------------
// 2. Comunicación con el servidor en C++
// ---------------------------------------------------------------------

async function llamarServidor(operacion, datos) {
    try {
        const respuesta = await fetch('/api/' + operacion, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(datos),
        });
        return await respuesta.json();
    } catch (error) {
        return { ok: false, error: 'No se pudo conectar con el servidor. Verifica que el programa siga abierto.' };
    }
}

// ---------------------------------------------------------------------
// 3. Navegación: la parte después de "#/" en la dirección decide la vista
// ---------------------------------------------------------------------

let rutaActual = 'inicio';

function mostrarVista() {
    const ruta = location.hash.replace('#/', '') || 'inicio';
    rutaActual = ruta;

    // Marcar la opción activa en el menú.
    document.querySelectorAll('.menu a').forEach(enlace => {
        enlace.classList.toggle('activo', enlace.dataset.ruta === ruta);
    });

    $('#vista-inicio').hidden = true;
    $('#vista-conteo').hidden = true;
    $('#vista-grafo').hidden = true;

    if (OPERACIONES_CONTEO[ruta]) {
        prepararVistaConteo(ruta);
        $('#vista-conteo').hidden = false;
    } else if (OPERACIONES_GRAFO[ruta]) {
        prepararVistaGrafo(ruta);
        $('#vista-grafo').hidden = false;
    } else {
        $('#vista-inicio').hidden = false;
    }
    window.scrollTo(0, 0);
}

// ---------------------------------------------------------------------
// 4. Permutaciones y combinaciones
// ---------------------------------------------------------------------

// Solo "permutación con repetición" tiene dos variantes: "posiciones" (nᵏ) y "elementos".
let varianteActual = 'posiciones';

function prepararVistaConteo(ruta) {
    const op = OPERACIONES_CONTEO[ruta];
    const tieneVariantes = Boolean(op.elementos);
    if (!tieneVariantes) varianteActual = 'posiciones';

    const usaElementos = varianteActual === 'elementos';
    const datos = usaElementos ? op.elementos : op;

    $('#conteo-grupo').textContent = op.grupo;
    $('#conteo-titulo').textContent = op.titulo;
    $('#conteo-descripcion').textContent = datos.descripcion;
    $('#conteo-formula').textContent = datos.formula;
    $('#etiqueta-n').textContent = op.etiquetaN;
    $('#etiqueta-k').textContent = op.etiquetaK;

    $('#conteo-variantes').hidden = !tieneVariantes;
    document.querySelectorAll('#conteo-variantes button').forEach(boton => {
        boton.setAttribute('aria-selected', String(boton.dataset.variante === varianteActual));
    });
    $('#campos-nk').hidden = usaElementos;
    $('#campos-elementos').hidden = !usaElementos;

    const ejemplo = $('#conteo-ejemplo');
    ejemplo.replaceChildren(crear('strong', '', 'Ejemplo: '), document.createTextNode(datos.ejemplo.texto));

    $('#resultado-conteo').replaceChildren();
}

function actualizarTipoElementos() {
    const tipo = document.querySelector('input[name="tipo-elementos"]:checked').value;
    const input = $('#valor-elementos');
    if (tipo === 'palabra') {
        $('#etiqueta-elementos').textContent = 'Palabra';
        input.placeholder = 'Ej. CASA';
        $('#ayuda-elementos').textContent = 'Se cuentan las letras que se repiten. Los espacios se ignoran.';
    } else {
        $('#etiqueta-elementos').textContent = 'Cuántas veces se repite cada elemento';
        input.placeholder = 'Ej. 2, 1, 1';
        $('#ayuda-elementos').textContent = 'Escribe las cantidades separadas por comas. Ej.: 3 rojas, 2 azules y 1 verde → 3, 2, 1';
    }
}

// Arma los datos que se envían al servidor según la variante elegida.
function datosConteo() {
    if (varianteActual === 'elementos' && OPERACIONES_CONTEO[rutaActual].elementos) {
        const tipo = document.querySelector('input[name="tipo-elementos"]:checked').value;
        const texto = $('#valor-elementos').value;
        if (tipo === 'palabra') return { palabra: texto };
        const cantidades = texto.split(',').filter(parte => parte.trim() !== '').map(Number);
        return { cantidades };
    }
    return { n: aNumero($('#valor-n').value), k: aNumero($('#valor-k').value) };
}

async function calcularConteo(evento) {
    evento.preventDefault();
    const op = OPERACIONES_CONTEO[rutaActual];
    const api = varianteActual === 'elementos' && op.elementos ? op.elementos.api : op.api;
    const boton = evento.submitter || $('#formulario-conteo button[type="submit"]');

    boton.disabled = true;
    const respuesta = await llamarServidor(api, datosConteo());
    boton.disabled = false;

    mostrarResultadoConteo(respuesta);
}

function cargarEjemploConteo() {
    const op = OPERACIONES_CONTEO[rutaActual];
    if (varianteActual === 'elementos' && op.elementos) {
        document.querySelector('input[name="tipo-elementos"][value="palabra"]').checked = true;
        actualizarTipoElementos();
        $('#valor-elementos').value = op.elementos.ejemplo.palabra;
    } else {
        $('#valor-n').value = op.ejemplo.n;
        $('#valor-k').value = op.ejemplo.k;
    }
    $('#formulario-conteo').requestSubmit();
}

function mostrarResultadoConteo(respuesta) {
    const contenedor = $('#resultado-conteo');
    if (!respuesta.ok) {
        contenedor.replaceChildren(crear('div', 'alerta', respuesta.error));
        return;
    }

    const tarjeta = crear('section', 'tarjeta resultado');
    tarjeta.appendChild(crear('p', 'resultado-etiqueta', 'Resultado'));

    const principal = crear('p', 'resultado-principal');
    principal.append(respuesta.simbolo + ' = ', crear('strong', '', formatearNumero(respuesta.resultado)));
    tarjeta.appendChild(principal);

    const digitos = respuesta.resultado.length;
    let nota = respuesta.resultado === '1' ? 'Hay 1 sola forma posible.' : 'Es el número de formas posibles.';
    if (digitos > 15) nota += ` Tiene ${formatearNumero(String(digitos))} dígitos.`;
    tarjeta.appendChild(crear('p', 'resultado-nota', nota));

    tarjeta.appendChild(crear('h3', '', 'Desarrollo paso a paso'));
    tarjeta.appendChild(crearListaPasos(respuesta.pasos));

    contenedor.replaceChildren(tarjeta);
    tarjeta.scrollIntoView({ behavior: 'smooth', block: 'nearest' });
}

function crearListaPasos(pasos) {
    const lista = crear('ol', 'pasos');
    for (const paso of pasos) {
        const item = crear('li');
        item.append(crear('span', 'paso-explicacion', paso.explicacion), crear('span', 'paso-expresion', paso.expresion));
        lista.appendChild(item);
    }
    return lista;
}

// ---------------------------------------------------------------------
// 5. Grafos
// ---------------------------------------------------------------------

// El grafo se comparte entre "ruta más corta" y "ruta más larga",
// así el usuario lo escribe una vez y puede calcular ambas.
let aristas = cargarAristasGuardadas();
let tipoGrafo = TIPOS_GRAFO[leerLocal('md-tipo-grafo')] ? leerLocal('md-tipo-grafo') : 'no-dirigido';

// Recupera el grafo guardado en el navegador; si no hay o está dañado, empieza con una fila vacía.
function cargarAristasGuardadas() {
    const guardadas = leerLocal('md-aristas');
    const valida = a => a && typeof a.origen === 'string' && typeof a.destino === 'string';
    if (Array.isArray(guardadas) && guardadas.length > 0 && guardadas.every(valida)) return guardadas;
    return [{ origen: '', destino: '', peso: 1, dobleSentido: true }];
}

function prepararVistaGrafo(ruta) {
    const op = OPERACIONES_GRAFO[ruta];
    $('#grafo-titulo').textContent = op.titulo;
    $('#grafo-descripcion').textContent = op.descripcion;
    $('#boton-calcular-grafo').textContent = op.boton;
    $('#resultado-grafo').replaceChildren();
    mostrarTipoGrafo();
    dibujarListaAristas();
    actualizarGrafo();
}

// Marca el botón del tipo elegido y cambia el texto de ayuda.
function mostrarTipoGrafo() {
    document.querySelectorAll('#tipo-grafo button').forEach(boton => {
        boton.setAttribute('aria-checked', String(boton.dataset.tipo === tipoGrafo));
    });
    $('#ayuda-tipo-grafo').textContent = TIPOS_GRAFO[tipoGrafo];
}

function cambiarTipoGrafo(tipo) {
    tipoGrafo = tipo;
    guardarLocal('md-tipo-grafo', tipoGrafo);
    mostrarTipoGrafo();
    dibujarListaAristas();   // la columna "Sentido" cambia según el tipo
    actualizarGrafo();
}

// Vuelve a crear las filas de la tabla de aristas a partir de la lista "aristas".
function dibujarListaAristas() {
    const lista = $('#lista-aristas');
    lista.replaceChildren();

    aristas.forEach((arista, i) => {
        const fila = crear('div', 'fila-arista');

        const origen = crear('input');
        origen.value = arista.origen;
        origen.placeholder = 'A';
        origen.setAttribute('aria-label', `Origen de la arista ${i + 1}`);
        origen.addEventListener('input', () => { arista.origen = origen.value.trim(); actualizarGrafo(); });

        // Solo en un grafo mixto se elige el sentido de cada arista;
        // en los demás se muestra fijo (— no dirigido, → dirigido).
        let sentido;
        if (tipoGrafo === 'mixto') {
            sentido = crear('select');
            sentido.setAttribute('aria-label', `Sentido de la arista ${i + 1}`);
            sentido.append(new Option('↔', 'doble'), new Option('→', 'uno'));
            sentido.value = arista.dobleSentido ? 'doble' : 'uno';
            sentido.addEventListener('change', () => { arista.dobleSentido = sentido.value === 'doble'; actualizarGrafo(); });
        } else {
            sentido = crear('span', 'sentido-fijo', tipoGrafo === 'dirigido' ? '→' : '—');
        }

        const destino = crear('input');
        destino.value = arista.destino;
        destino.placeholder = 'B';
        destino.setAttribute('aria-label', `Destino de la arista ${i + 1}`);
        destino.addEventListener('input', () => { arista.destino = destino.value.trim(); actualizarGrafo(); });

        const peso = crear('input');
        peso.type = 'number';
        peso.min = '0';
        peso.step = '1';
        peso.inputMode = 'numeric';
        peso.value = arista.peso;
        peso.setAttribute('aria-label', `Peso de la arista ${i + 1}`);
        peso.addEventListener('input', () => { arista.peso = peso.value; actualizarGrafo(); });

        const eliminar = crear('button', 'boton-eliminar', '×');
        eliminar.type = 'button';
        eliminar.title = 'Eliminar arista';
        eliminar.setAttribute('aria-label', `Eliminar arista ${i + 1}`);
        eliminar.addEventListener('click', () => {
            aristas.splice(i, 1);
            dibujarListaAristas();
            actualizarGrafo();
        });

        fila.append(origen, sentido, destino, peso, eliminar);
        lista.appendChild(fila);
    });
}

// Se llama cada vez que cambia el grafo: guarda, actualiza las listas de nodos y redibuja.
// El resultado anterior se borra porque ya no corresponde al grafo nuevo.
function actualizarGrafo() {
    guardarLocal('md-aristas', aristas);
    const nodos = obtenerNodos(aristas);
    llenarSelectorNodos($('#grafo-origen'), nodos, 0);
    llenarSelectorNodos($('#grafo-destino'), nodos, nodos.length - 1);
    dibujarGrafo($('#dibujo-grafo'), aristas, tipoGrafo, null);
    $('#resultado-grafo').replaceChildren();
}

// Llena un <select> con los nodos, conservando la opción elegida si todavía existe.
function llenarSelectorNodos(selector, nodos, posicionInicial) {
    const elegido = selector.value;
    selector.replaceChildren(...nodos.map(n => new Option(n, n)));
    if (nodos.includes(elegido)) selector.value = elegido;
    else if (nodos.length > 0) selector.value = nodos[Math.max(0, posicionInicial)];
}

function agregarArista() {
    aristas.push({ origen: '', destino: '', peso: 1, dobleSentido: true });
    dibujarListaAristas();
    actualizarGrafo();
    const filas = document.querySelectorAll('.fila-arista');
    filas[filas.length - 1].querySelector('input').focus();
}

function cargarEjemploGrafo() {
    aristas = EJEMPLO_GRAFO.aristas.map(a => ({ ...a }));
    cambiarTipoGrafo(EJEMPLO_GRAFO.tipo);
    $('#grafo-origen').value = EJEMPLO_GRAFO.origen;
    $('#grafo-destino').value = EJEMPLO_GRAFO.destino;
}

function limpiarGrafo() {
    aristas = [{ origen: '', destino: '', peso: 1, dobleSentido: true }];
    dibujarListaAristas();
    actualizarGrafo();
}

async function calcularGrafo(evento) {
    evento.preventDefault();
    const op = OPERACIONES_GRAFO[rutaActual];

    // Las filas completamente vacías se ignoran; las demás las valida el servidor.
    const aristasUsadas = aristas
        .filter(a => a.origen !== '' || a.destino !== '')
        .map(a => ({ origen: a.origen, destino: a.destino, peso: aNumero(a.peso), dobleSentido: a.dobleSentido }));

    const boton = $('#boton-calcular-grafo');
    boton.disabled = true;
    const respuesta = await llamarServidor(op.api, {
        aristas: aristasUsadas,
        tipo: tipoGrafo,
        origen: $('#grafo-origen').value,
        destino: $('#grafo-destino').value,
    });
    boton.disabled = false;

    mostrarResultadoGrafo(respuesta);
}

function mostrarResultadoGrafo(respuesta) {
    const contenedor = $('#resultado-grafo');
    if (!respuesta.ok) {
        dibujarGrafo($('#dibujo-grafo'), aristas, tipoGrafo, null);
        contenedor.replaceChildren(crear('div', 'alerta', respuesta.error));
        return;
    }

    const esCorta = rutaActual === 'ruta-mas-corta';
    const resaltarRuta = (nodos, pesos) => dibujarGrafo($('#dibujo-grafo'), aristas, tipoGrafo, { nodos, pesos });
    resaltarRuta(respuesta.ruta, respuesta.pesos);

    const tarjeta = crear('section', 'tarjeta resultado');
    tarjeta.appendChild(crear('p', 'resultado-etiqueta', esCorta ? 'Ruta más corta' : 'Ruta más larga'));

    const principal = crear('p', 'resultado-principal');
    principal.append('Peso total = ', crear('strong', '', formatearNumero(String(respuesta.total))));
    tarjeta.appendChild(principal);

    const chips = crear('div', 'ruta-chips');
    respuesta.ruta.forEach((nodo, i) => {
        if (i > 0) chips.appendChild(crear('span', 'chip-flecha', '→'));
        chips.appendChild(crear('span', 'chip', nodo));
    });
    tarjeta.appendChild(chips);
    tarjeta.appendChild(crear('p', 'resultado-nota', 'Suma de los pesos: ' + respuesta.detalle));

    tarjeta.appendChild(crear('h3', '', 'Desarrollo paso a paso'));
    tarjeta.appendChild(crearListaPasos(respuesta.pasos));

    if (esCorta) {
        tarjeta.appendChild(crear('h3', '', 'Tabla final de Dijkstra'));
        tarjeta.appendChild(crearTablaDijkstra(respuesta.tabla, respuesta.ruta));
    } else {
        tarjeta.appendChild(crear('h3', '', 'Todos los caminos encontrados'));
        tarjeta.appendChild(crearTablaCaminos(respuesta, resaltarRuta));
    }

    contenedor.replaceChildren(tarjeta);
    tarjeta.scrollIntoView({ behavior: 'smooth', block: 'nearest' });
}

function crearTabla(encabezados) {
    const contenedor = crear('div', 'tabla-contenedor');
    const tabla = crear('table', 'tabla');
    const filaEncabezado = crear('tr');
    for (const [texto, clase] of encabezados) filaEncabezado.appendChild(crear('th', clase || '', texto));
    tabla.appendChild(crear('thead')).appendChild(filaEncabezado);
    const cuerpo = tabla.appendChild(crear('tbody'));
    contenedor.appendChild(tabla);
    return { contenedor, cuerpo };
}

function crearTablaDijkstra(filas, ruta) {
    const { contenedor, cuerpo } = crearTabla([['Nodo'], ['Distancia desde el origen', 'numero'], ['Anterior']]);
    for (const fila of filas) {
        const tr = crear('tr', ruta.includes(fila.nodo) ? 'en-ruta' : '');
        tr.append(crear('td', '', fila.nodo), crear('td', 'numero', fila.distancia), crear('td', '', fila.anterior));
        cuerpo.appendChild(tr);
    }
    const nota = crear('p', 'nota-tabla', 'Las filas resaltadas son los nodos de la ruta. "∞" significa que no se puede llegar a ese nodo.');
    contenedor.appendChild(nota);
    return contenedor;
}

// Tabla de caminos: al hacer clic en una fila, ese camino se resalta en el dibujo.
function crearTablaCaminos(respuesta, resaltarRuta) {
    const { contenedor, cuerpo } = crearTabla([['#'], ['Camino'], ['Suma de pesos'], ['Total', 'numero']]);
    respuesta.caminos.forEach((camino, i) => {
        const tr = crear('tr', 'seleccionable' + (i === 0 ? ' seleccionado' : ''));
        tr.tabIndex = 0;
        tr.append(
            crear('td', '', String(i + 1)),
            crear('td', '', camino.nodos.join(' → ')),
            crear('td', '', camino.detalle),
            crear('td', 'numero', String(camino.total)),
        );
        const seleccionar = () => {
            cuerpo.querySelectorAll('tr').forEach(fila => fila.classList.remove('seleccionado'));
            tr.classList.add('seleccionado');
            resaltarRuta(camino.nodos, camino.pesos);
        };
        tr.addEventListener('click', seleccionar);
        tr.addEventListener('keydown', e => { if (e.key === 'Enter' || e.key === ' ') { e.preventDefault(); seleccionar(); } });
        cuerpo.appendChild(tr);
    });

    let texto = 'Haz clic en un camino para verlo en el dibujo. Están ordenados de mayor a menor.';
    if (respuesta.totalCaminos > respuesta.caminos.length) {
        texto += ` Se muestran los ${respuesta.caminos.length} mayores de ${formatearNumero(String(respuesta.totalCaminos))} caminos.`;
    }
    contenedor.appendChild(crear('p', 'nota-tabla', texto));
    return contenedor;
}

// ---------------------------------------------------------------------
// 6. Tema claro / oscuro
// ---------------------------------------------------------------------

function temaEfectivo() {
    const elegido = document.documentElement.dataset.tema;
    if (elegido) return elegido;
    return matchMedia('(prefers-color-scheme: dark)').matches ? 'oscuro' : 'claro';
}

function actualizarBotonTema() {
    $('#boton-tema').textContent = temaEfectivo() === 'oscuro' ? '☀ Tema claro' : '☾ Tema oscuro';
}

function cambiarTema() {
    const nuevo = temaEfectivo() === 'oscuro' ? 'claro' : 'oscuro';
    document.documentElement.dataset.tema = nuevo;
    guardarLocal('md-tema', nuevo);
    actualizarBotonTema();
}

// ---------------------------------------------------------------------
// Inicio: conectar botones y formularios
// ---------------------------------------------------------------------

function iniciar() {
    const temaGuardado = leerLocal('md-tema');
    if (temaGuardado) document.documentElement.dataset.tema = temaGuardado;
    actualizarBotonTema();
    $('#boton-tema').addEventListener('click', cambiarTema);

    // Permutaciones y combinaciones
    $('#formulario-conteo').addEventListener('submit', calcularConteo);
    $('#formulario-conteo').addEventListener('reset', () => {
        $('#resultado-conteo').replaceChildren();
        setTimeout(actualizarTipoElementos);   // espera a que el navegador termine de limpiar el formulario
    });
    $('#boton-ejemplo-conteo').addEventListener('click', cargarEjemploConteo);
    document.querySelectorAll('input[name="tipo-elementos"]').forEach(radio => {
        radio.addEventListener('change', () => { actualizarTipoElementos(); $('#valor-elementos').value = ''; });
    });
    document.querySelectorAll('#conteo-variantes button').forEach(boton => {
        boton.addEventListener('click', () => {
            varianteActual = boton.dataset.variante;
            prepararVistaConteo(rutaActual);
        });
    });

    // Grafos
    $('#formulario-grafo').addEventListener('submit', calcularGrafo);
    document.querySelectorAll('#tipo-grafo button').forEach(boton => {
        boton.addEventListener('click', () => cambiarTipoGrafo(boton.dataset.tipo));
    });
    $('#boton-agregar-arista').addEventListener('click', agregarArista);
    $('#boton-ejemplo-grafo').addEventListener('click', cargarEjemploGrafo);
    $('#boton-limpiar-grafo').addEventListener('click', limpiarGrafo);
    $('#grafo-origen').addEventListener('change', actualizarGrafo);
    $('#grafo-destino').addEventListener('change', actualizarGrafo);

    window.addEventListener('hashchange', mostrarVista);
    mostrarVista();
}

iniciar();
