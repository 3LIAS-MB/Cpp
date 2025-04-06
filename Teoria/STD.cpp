// 📌 Contenedores (Containers)

//     std::vector: Arreglo dinámico (el contenedor más usado).

//     std::array: Arreglo de tamaño fijo (mejor que los arrays estilo C).

//     std::list / std::forward_list: Listas enlazadas.

//     std::deque: Cola de doble extremo.

//     std::stack / std::queue / std::priority_queue: Adaptadores para pilas y colas.

//     std::set / std::multiset: Conjuntos ordenados (sin duplicados / con duplicados).

//     std::map / std::multimap: Diccionarios ordenados (clave-valor).

//     std::unordered_set / std::unordered_map: Versiones no ordenadas (hash tables).

// 🔍 Algoritmos (Algorithms)

//     std::sort: Ordenamiento eficiente (QuickSort, HeapSort, etc.).

//     std::find / std::binary_search: Búsqueda en contenedores.

//     std::transform: Aplica una función a un rango.

//     std::copy / std::move: Copia o mueve elementos.

//     std::accumulate: Suma o reduce un rango.

//     std::max_element / std::min_element: Encuentra el máximo/mínimo.

// 🔄 Iteradores (Iterators)

//     std::begin / std::end: Obtienen iteradores de contenedores.

//     std::advance / std::next / std::prev: Navegación entre iteradores.

//     std::reverse_iterator: Itera en orden inverso.

// 📚 Utilidades (Utilities)

//     std::pair: Almacena dos valores (como una tupla de 2 elementos).

//     std::tuple: Almacena múltiples valores (generalización de pair).

//     std::optional (C++17): Representa un valor que puede estar ausente.

//     std::variant (C++17): Almacena uno de varios tipos posibles.

//     std::any (C++17): Almacena cualquier tipo de dato.

// ⏱ Manejo de tiempo (Chrono)

//     std::chrono::system_clock: Para trabajar con tiempo del sistema.

//     std::chrono::duration: Representa intervalos de tiempo.

//     std::chrono::time_point: Punto específico en el tiempo.

// 📦 Smart Pointers (Gestión de memoria)

//     std::unique_ptr: Puntero único (no copiable, solo movable).

//     std::shared_ptr: Puntero con conteo de referencias.

//     std::weak_ptr: Puntero débil (evita ciclos con shared_ptr).

// 🔤 Strings y streams

//     std::string: Cadena de caracteres dinámica.

//     std::string_view (C++17): Vista de una cadena sin copiarla.

//     std::istream / std::ostream: Para entrada/salida (como std::cin y std::cout).

// 🧵 Concurrencia (Multithreading)

//     std::thread: Hilos de ejecución.

//     std::mutex / std::lock_guard: Sincronización.

//     std::future / std::async: Trabajo asíncrono.

//     std::atomic: Operaciones atómicas.

// 🎯 Otros importantes

//     std::function: Almacena cualquier función, lambda o funtor.

//     std::move / std::forward: Semántica de movimiento y perfect forwarding.

//     std::initializer_list: Para inicializar contenedores con {}.