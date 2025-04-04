/*************************************************************
 *               FUNDAMENTOS DE C++ - GUÍA COMPLETA          *
 *************************************************************/

 //==================== 1. ESTRUCTURA BÁSICA ====================
 #include <iostream>  // Biblioteca estándar para entrada/salida

 /*
 Todo programa en C++ necesita:
 1. #includes (bibliotecas)
 2. Función main() (punto de entrada)
 */
 int main() {
     std::cout << "¡Hola, C++!\n";
     return 0;  // Retorno exitoso
 }
 
 //=============== 2. VARIABLES Y CONSTANTES ====================
 /*
 Variables: Almacenan datos modificables
 Constantes: Valores inmutables (3 tipos)
 */
 int variable = 10;               // Variable normal
 const float PI = 3.1416f;        // Constante en tiempo de ejecución
 constexpr int MAX = 100;         // Constante en tiempo de compilación (C++11)
 #define VIEJA_MAX 100            // Macro (evitar en C++ moderno)
 
 //=============== 3. TIPOS DE DATOS FUNDAMENTALES ==============
 /*
 Tipos primitivos:
 - Enteros: short(2b), int(4b), long(4/8b), long long(8b)
 - Decimales: float(4b), double(8b), long double(16b)
 - Carácter: char(1b), wchar_t(2/4b)
 - Booleano: bool (true/false)
 */
 int entero = 42;
 double decimal = 3.1416;
 char caracter = 'A';
 bool logico = true;
 
 // Tipos compuestos:
 auto automatico = 3.14;          // Deduce tipo (C++11)
 decltype(decimal) copia_tipo;    // Copia tipo de otra variable
 
 //=============== 4. OPERADORES ================================
 // Aritméticos: +, -, *, /, %, ++, --
 // Comparación: ==, !=, <, >, <=, >=
 // Lógicos: && (AND), || (OR), ! (NOT)
 // Bitwise: &, |, ^, ~, <<, >> (manipulación de bits)
 
 int a = 10, b = 3;
 double division = static_cast<double>(a) / b;  // Conversión explícita
 
 //=============== 5. ESTRUCTURAS DE CONTROL ====================
 // If-else con inicialización (C++17)
 if (int resultado = a * b; resultado > 20) {
     std::cout << "Resultado grande: " << resultado << "\n";
 }
 
 // Switch mejorado
 enum Dia { LUN, MAR, MIE, JUE, VIE, SAB, DOM };
 Dia hoy = MIE;
 
 switch (hoy) {
 case LUN:
     std::cout << "Inicio de semana\n";
     break;
 case VIE:
     std::cout << "¡Viernes!\n";
     break;
 
 default: std::cout << "Día normal\n";
 }
 
 //=============== 6. BUCLES ====================================
 // While (primero verifica condición)
 int i = 0;
 while (i < 3) {
     std::cout << i++ << " ";
 }
 
 // Do-while (ejecuta al menos una vez)
 do {
     std::cout << "Se ejecuta siempre\n";
 } while (false);
 
 // For tradicional y range-based (C++11)
 for (int j = 0; j < 5; j++) {
     std::cout << j << " ";
 }
 
 int numeros[] = { 1, 2, 3 };
 for (int num : numeros) {       // Para cada elemento
     std::cout << num << " ";
 }
 
 //=============== 7. FUNCIONES ================================
 // Función con parámetros opcionales
 void saludar(std::string nombre = "Usuario") {
     std::cout << "Hola, " << nombre << "!\n";
 }
 
 saludar("Carlos");
 
 // Sobrecarga de funciones
 int sumar(int a, int b) { return a + b; }
 double sumar(double a, double b) { return a + b; }
 
 // Función lambda (C++11)
 // Una función lambda en C++ es una función anónima que se puede definir 
 // en línea dentro del código. Las lambdas son útiles para operaciones 
 // rápidas y sencillas que no requieren una función completa con nombre.
 
 //La sintaxis básica de una lambda es la siguiente :
 //[captura](parametros) -> tipo_retorno { cuerpo };
 
 auto cuadrado = [](int x) { return x * x; };
 int resultado = cuadrado(5);  // resultado será 25
 
 std::cout << "El cuadrado de 5 es: " << resultado << "\n";
 
 //=============== 8. ARREGLOS Y STRINGS ========================
 // Arreglo estático (bidimensional)
 
 // Este arreglo se almacena en la memoria de manera contigua y su tamaño es
 // fijo, lo que significa que no puede cambiar durante la ejecución del programa.
 
 int matriz[2][3] = { {1, 2, 3}, {4, 5, 6} };
 
 // Arreglos Dinámicos : Utilizan punteros y memoria dinámica para
 // permitir tamaños variables.Se gestionan con new y delete.
 int* arreglo = new int[10];  // Arreglo dinámico de 10 enteros
 delete[] arreglo;            // Liberar memoria
 
 // Usando std::array (C++11)
 #include <array>
 std::array<int, 5> arr_moderno = { 1, 2, 3, 4, 5 };
 
 // Usando std::vector (C++11)
 #include <vector>
 std::vector<int> vec = { 1, 2, 3, 4, 5 };
 
 // Usando std::initializer_list (C++11)
 #include <initializer_list>
 std::initializer_list<int> init_list = { 1, 2, 3, 4, 5 };
 
 // Usando std::valarray (C++11)
 #include <valarray>
 std::valarray<int> val_arr = { 1, 2, 3, 4, 5 };
 
 // STL Containers (C++11)
 
 // Los contenedores de la STL(Standard Template Library) en C++ son clases que proporcionan estructuras
 // de datos genéricas.Estos contenedores permiten almacenar y manipular colecciones de datos de manera
 // eficiente y segura.Algunos de los contenedores más comunes incluyen:
 // 
 //•	std::array : Un contenedor de tamaño fijo que encapsula un arreglo C.
 //•	std::vector : Un arreglo dinámico que puede cambiar de tamaño.
 //•	std::deque : Un arreglo doblemente terminado que permite inserciones y eliminaciones eficientes en ambos extremos.
 //•	std::list : Una lista enlazada doblemente.
 //•	std::set : Un conjunto de elementos únicos, ordenados automáticamente.
 //•	std::map : Un contenedor de pares clave - valor, ordenado automáticamente por las claves.
 //•	std::unordered_set : Similar a std::set, pero no garantiza el orden de los elementos.
 //•	std::unordered_map : Similar a std::map, pero no garantiza el orden de los elementos.
 // 
 // Estos contenedores son parte de la biblioteca estándar de C++ y se encuentran en el espacio de nombres std.
 // Proporcionan una interfaz uniforme y son altamente optimizados para diferentes tipos de operaciones.
 
 #include <array>
 #include <vector>
 #include <string>
 
 std::array<int, 5> arr_moderno = { 1, 2, 3, 4, 5 };
 std::vector<double> dinamico = { 1.1, 2.2 };  // Tamaño variable
 std::string texto = "C++ Moderno";
 
 //=============== 9. ESTRUCTURAS Y CLASES ======================
 // 
 //Estructuras(struct)
 
 // Las estructuras en C++ se utilizan para agrupar diferentes tipos de datos bajo un mismo nombre.
 // Son útiles para representar objetos simples que no requieren encapsulamiento o funcionalidad avanzada.
 // 
 // Características:
 // • Por defecto, todos los miembros de una estructura son públicos.
 // • Se utilizan principalmente para agrupar datos relacionados.
 
 struct Punto {
     double x, y;
 
     // Constructor
     Punto(double x, double y) : x(x), y(y) {}
 
     // Método
     void mostrar() const {
         std::cout << "(" << x << ", " << y << ")\n";
     }
 };
 
 // Clases(class)
 
 // Las clases en C++ son una extensión de las estructuras y se utilizan para la programación
 // orientada a objetos.Permiten encapsular datos y funciones que operan sobre esos datos,
 // proporcionando un mayor control sobre el acceso y la manipulación de los mismos.
 // 
 // Características:
 // • Por defecto, todos los miembros de una clase son privados.
 // • Soportan características avanzadas como herencia, polimorfismo y encapsulamiento.
 // • Permiten definir métodos(funciones miembro) y controlar el acceso a los datos 
 //   mediante modificadores de acceso(public, private, protected).
 class Rectangulo {
 private:
     double ancho, alto;
 public:
     // Constructor
     Rectangulo(double a, double h) : ancho(a), alto(h) {}
 
     // Métodos
     double area() const { return ancho * alto; }
     void escalar(double factor) { ancho *= factor; alto *= factor; }
 };
 
 //=============== 10. PROGRAMACIÓN ORIENTADA A OBJETOS =========
 // Herencia
 class Animal {
 public:
     virtual void sonar() { std::cout << "Sonido indefinido\n"; }
     virtual ~Animal() = default;  // Destructor virtual
 };
 
 class Gato : public Animal {
 public:
     void sonar() override { std::cout << "Miau!\n"; }
 };
 
 // Polimorfismo
 void hacerSonar(Animal & animal) {
     animal.sonar();  // Llamada polimórfica
 }
 
 //=============== 11. MANEJO DE MEMORIA ========================
 // Punteros inteligentes (C++11)
 #include <memory>
 auto ptr = std::make_unique<int>(42);      // Unique pointer (no compartido)
 auto shared_ptr = std::make_shared<int>(10); // Shared pointer
 
 //=============== 12. PLANTILLAS (TEMPLATES) ===================
 template <typename T>
 T maximo(T a, T b) {
     return (a > b) ? a : b;
 }
 
 //=============== 13. MANEJO DE ERRORES ========================
 try {
     if (b == 0) throw std::runtime_error("División por cero");
     double resultado = a / b;
 }
 catch (const std::exception& e) {
     std::cerr << "Error: " << e.what() << "\n";
 }
 
 /*************************************************************
  *                 BUENAS PRÁCTICAS - RESUMEN                *
  *************************************************************/
  /*
  1. Usar nombres descriptivos para variables/funciones
  2. Preferir const/constexpr sobre #define
  3. Usar nullptr en lugar de NULL o 0
  4. Inicializar siempre las variables
  5. Usar range-based for cuando sea posible
  6. Preferir std::vector sobre arreglos crudos
  7. Usar punteros inteligentes para manejo de memoria
  8. Documentar código con comentarios claros
  9. Seguir la regla de 0/3/5 para clases
  10. Validar entradas de usuario/archivos
  */
 
  // Compilación recomendada:
  // g++ -std=c++17 -Wall -Wextra -pedantic programa.cpp -o programa