#ifndef COMMON_H
#define COMMON_H

#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <string>

// Constantes de simulación
constexpr int NSTEPS = 1000;    // Número de pasos de simulación
constexpr int SAVEFREQ = 10;    // Frecuencia de guardado
constexpr double DENSITY = 0.0005;
constexpr double MASS = 0.01;
constexpr double CUTOFF = 0.01;
constexpr double MIN_R = CUTOFF / 100;
constexpr double DT = 0.0005;

// Estructura de una partícula
struct Particle {
    double x, y;      // Posición
    double vx, vy;    // Velocidad
    double ax, ay;    // Aceleración
};

// Funciones de simulación
void set_size(int n);
void init_particles(int n, Particle *p);
void apply_force(Particle &a, Particle &b);
void move(Particle &p);
void save(FILE *f, int n, Particle *p);

// Funciones de utilidad
double read_timer();
int find_option(int argc, char **argv, const char *option);
int read_int(int argc, char **argv, const char *option, int default_val);
std::string read_string(int argc, char **argv, const char *option, const char *default_val);

#endif