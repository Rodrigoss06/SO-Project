/*
 * race_condition.c
 * Sesión 3 — Actividad A2: condición de carrera y corrección con mutex.
 *
 * Lanza 4 hilos que incrementan 100 000 veces un contador global
 * (esperado 400 000). Sin protección, `contador++` no es atómico
 * (lee -> incrementa -> escribe) y dos hilos pueden pisarse, dando un
 * resultado final menor al esperado y distinto en cada ejecución. Con
 * `pthread_mutex_lock/unlock` la sección crítica queda protegida y el
 * resultado siempre es 400000.
 *
 * Uso:
 *   ./race_condition          -> sin mutex (demuestra la carrera)
 *   ./race_condition mutex    -> con mutex (resultado correcto)
 *
 * Compilar: make s3
 *   (equivalente a: gcc -Wall -Wextra -std=c11 -pthread
 *    sesion3-hilos/race_condition.c -o bin/race_condition)
 */

#include <pthread.h>
#include <stdio.h>
#include <string.h>

#define NUM_HILOS 4
#define INCREMENTOS 100000

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

int main(int argc, char *argv[]) {
    usar_mutex = (argc > 1 && strcmp(argv[1], "mutex") == 0);

    pthread_t hilos[NUM_HILOS];

    for (int i = 0; i < NUM_HILOS; i++) {
        pthread_create(&hilos[i], NULL, incrementar, NULL);
    }

    for (int i = 0; i < NUM_HILOS; i++) {
        pthread_join(hilos[i], NULL);
    }

    printf("Modo: %s\n", usar_mutex ? "con mutex" : "sin mutex");
    printf("Contador final: %ld (esperado %d)\n",
           contador, NUM_HILOS * INCREMENTOS);

    return 0;
}
