/*
 * s3_72_tiempo.c
 * Ejercicio 7.2 — Medición de tiempo con y sin mutex.
 *
 * Variante de sesion3-hilos/race_condition.c: 4 hilos incrementan
 * INCREMENTOS veces un contador global (esperado 4 * INCREMENTOS).
 * Se ejecuta dos veces -- sin mutex y con mutex -- midiendo el tiempo
 * de pared con clock_gettime(CLOCK_MONOTONIC) y comparando el resultado
 * final del contador contra el valor esperado.
 *
 * Compilar: make ejercicios
 *   (equivalente a: gcc -Wall -Wextra -std=c11 -pthread
 *    ejercicios/s3_72_tiempo.c -o bin/s3_72_tiempo)
 */

#include <pthread.h>
#include <stdio.h>
#include <time.h>

#define NUM_HILOS 4
#define INCREMENTOS 1000000

static long contador = 0;
static int usar_mutex = 0;
static pthread_mutex_t mutex_contador = PTHREAD_MUTEX_INITIALIZER;

static void *incrementar(void *arg) {
    (void) arg;

    for (int i = 0; i < INCREMENTOS; i++) {
        if (usar_mutex) {
            pthread_mutex_lock(&mutex_contador);
            contador++;
            pthread_mutex_unlock(&mutex_contador);
        } else {
            contador++;
        }
    }

    return NULL;
}

static double diferencia_seg(struct timespec inicio, struct timespec fin) {
    return (double) (fin.tv_sec - inicio.tv_sec)
         + (double) (fin.tv_nsec - inicio.tv_nsec) / 1e9;
}

static double ejecutar(int con_mutex, long *resultado) {
    contador = 0;
    usar_mutex = con_mutex;

    pthread_t hilos[NUM_HILOS];
    struct timespec inicio, fin;

    clock_gettime(CLOCK_MONOTONIC, &inicio);

    for (int i = 0; i < NUM_HILOS; i++) {
        pthread_create(&hilos[i], NULL, incrementar, NULL);
    }
    for (int i = 0; i < NUM_HILOS; i++) {
        pthread_join(hilos[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &fin);

    *resultado = contador;
    return diferencia_seg(inicio, fin);
}

int main(void) {
    long esperado = (long) NUM_HILOS * INCREMENTOS;
    long resultado_sin, resultado_con;

    double tiempo_sin = ejecutar(0, &resultado_sin);
    double tiempo_con = ejecutar(1, &resultado_con);

    printf("=== Medición de tiempo: %d hilos x %d incrementos ===\n\n",
           NUM_HILOS, INCREMENTOS);
    printf("%-12s %12s %12s %10s\n", "Modo", "Contador", "Esperado", "Tiempo (s)");
    printf("%-12s %12ld %12ld %10.4f", "sin mutex", resultado_sin, esperado, tiempo_sin);
    printf("%s\n", (resultado_sin != esperado) ? "  <-- condición de carrera" : "");
    printf("%-12s %12ld %12ld %10.4f", "con mutex", resultado_con, esperado, tiempo_con);
    printf("%s\n", (resultado_con != esperado) ? "  <-- INESPERADO" : "");

    printf("\nConclusión: sin mutex el contador final suele ser menor que el\n");
    printf("esperado (incrementos perdidos por la condición de carrera) y la\n");
    printf("ejecución es más rápida porque no hay espera por el lock. Con\n");
    printf("mutex el resultado es siempre correcto (%ld), a costa de un\n", esperado);
    printf("tiempo mayor por la sincronización de la sección crítica.\n");
    printf("Diferencia de tiempo (con - sin) = %.4f s\n", tiempo_con - tiempo_sin);

    return 0;
}
