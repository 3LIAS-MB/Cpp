#include "common.h"
#include <random>
#include <chrono>

double size;  // Tamaño del espacio de simulación

// Generador de números aleatorios en C++
std::mt19937 gen(std::random_device{}());
std::uniform_real_distribution<double> dist(-1.0, 1.0);

// Temporizador
double read_timer() {
    static auto start = std::chrono::steady_clock::now();
    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(end - start).count();
}

// Ajusta el tamaño del espacio según el número de partículas
void set_size(int n) {
    size = std::sqrt(DENSITY * n);
}

// Inicializa partículas con posiciones y velocidades aleatorias
void init_particles(int n, Particle *p) {
    int sx = static_cast<int>(std::ceil(std::sqrt(n)));
    int sy = (n + sx - 1) / sx;

    for (int i = 0; i < n; i++) {
        p[i].x = size * (1 + (i % sx)) / (1 + sx);
        p[i].y = size * (1 + (i / sx)) / (1 + sy);
        p[i].vx = dist(gen);
        p[i].vy = dist(gen);
        p[i].ax = 0;
        p[i].ay = 0;
    }
}

// Calcula la fuerza entre dos partículas
void apply_force(Particle &a, Particle &b) {
    double dx = b.x - a.x;
    double dy = b.y - a.y;
    double r2 = dx * dx + dy * dy;

    if (r2 > CUTOFF * CUTOFF) return;
    r2 = std::max(r2, MIN_R * MIN_R);
    double r = std::sqrt(r2);

    double coef = (1 - CUTOFF / r) / r2 / MASS;
    a.ax += coef * dx;
    a.ay += coef * dy;
}

// Mueve una partícula según su velocidad y aceleración
void move(Particle &p) {
    p.vx += p.ax * DT;
    p.vy += p.ay * DT;
    p.x += p.vx * DT;
    p.y += p.vy * DT;

    // Rebotar en los bordes
    if (p.x < 0 || p.x > size) {
        p.x = (p.x < 0) ? -p.x : 2 * size - p.x;
        p.vx = -p.vx;
    }
    if (p.y < 0 || p.y > size) {
        p.y = (p.y < 0) ? -p.y : 2 * size - p.y;
        p.vy = -p.vy;
    }
}

// Guarda las posiciones en un archivo
void save(FILE *f, int n, Particle *p) {
    static bool first = true;
    if (first) {
        fprintf(f, "%d %g\n", n, size);
        first = false;
    }
    for (int i = 0; i < n; i++) {
        fprintf(f, "%g %g\n", p[i].x, p[i].y);
    }
}