/*
 * hilos_basico.c
 * Sesión 3 — Actividad A1: hilos básicos con pthreads.
 *
 * Lanza 4 hilos; cada uno recibe su ID por puntero a memoria propia
 * (un elemento distinto de un arreglo, nunca `&i` de un bucle) e
 * imprime "Hilo i iniciado" / "Hilo i finalizado". El hilo principal
 * espera a todos con pthread_join.
 *
 * Compilar: make s3
 *   (equivalente a: gcc -Wall -Wextra -std=c11 -pthread
 *    sesion3-hilos/hilos_basico.c -o bin/hilos_basico)
 */

#include <pthread.h>
#include <stdio.h>

#define NUM_HILOS 4

static void *tarea(void *arg) {
    int id = *(int *) arg;

    printf("Hilo %d iniciado\n", id);
    printf("Hilo %d finalizado\n", id);

    return NULL;
}

int main(void) {
    pthread_t hilos[NUM_HILOS];
    int ids[NUM_HILOS];

    for (int i = 0; i < NUM_HILOS; i++) {
        ids[i] = i + 1;
        pthread_create(&hilos[i], NULL, tarea, &ids[i]);
    }

    for (int i = 0; i < NUM_HILOS; i++) {
        pthread_join(hilos[i], NULL);
    }

    return 0;
}
