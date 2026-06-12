/*
 * productor_consumidor.c
 * Sesión 3 — Actividad A3: productor-consumidor con buffer circular.
 *
 * 1 productor genera 20 números aleatorios (1-100) y los coloca en un
 * buffer circular de tamaño 5 (espera si está lleno); 1 consumidor
 * retira los 20 (espera si está vacío). Sincronización con
 * sem_t vacios = 5, llenos = 0 y un mutex para el acceso al buffer.
 * Cada inserción/extracción se registra junto con su posición en el
 * buffer.
 *
 * Orden de operaciones (evita el deadlock mutex/semáforo invertidos):
 *   productor:  sem_wait(vacios) -> lock -> ... -> unlock -> sem_post(llenos)
 *   consumidor: sem_wait(llenos) -> lock -> ... -> unlock -> sem_post(vacios)
 *
 * Uso: ./productor_consumidor [--seed N]
 *
 * Compilar: make s3
 *   (equivalente a: gcc -Wall -Wextra -std=c11 -pthread
 *    sesion3-hilos/productor_consumidor.c -o bin/productor_consumidor)
 */

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TAM_BUFFER 5
#define NUM_ITEMS 20

static int buffer[TAM_BUFFER];
static int pos_in = 0;
static int pos_out = 0;

static sem_t vacios;
static sem_t llenos;
static pthread_mutex_t mutex_buffer = PTHREAD_MUTEX_INITIALIZER;

static void *productor(void *arg) {
    (void) arg;

    for (int i = 0; i < NUM_ITEMS; i++) {
        int valor = 1 + rand() % 100;

        sem_wait(&vacios);
        pthread_mutex_lock(&mutex_buffer);

        buffer[pos_in] = valor;
        printf("Productor  -> coloca %3d en posición %d\n", valor, pos_in);
        pos_in = (pos_in + 1) % TAM_BUFFER;

        pthread_mutex_unlock(&mutex_buffer);
        sem_post(&llenos);
    }

    return NULL;
}

static void *consumidor(void *arg) {
    (void) arg;

    for (int i = 0; i < NUM_ITEMS; i++) {
        sem_wait(&llenos);
        pthread_mutex_lock(&mutex_buffer);

        int valor = buffer[pos_out];
        printf("Consumidor -> retira %3d de posición %d\n", valor, pos_out);
        pos_out = (pos_out + 1) % TAM_BUFFER;

        pthread_mutex_unlock(&mutex_buffer);
        sem_post(&vacios);
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    unsigned int semilla = (unsigned int) time(NULL);

    if (argc == 3 && strcmp(argv[1], "--seed") == 0) {
        semilla = (unsigned int) atoi(argv[2]);
    }
    srand(semilla);

    sem_init(&vacios, 0, TAM_BUFFER);
    sem_init(&llenos, 0, 0);

    pthread_t hilo_productor;
    pthread_t hilo_consumidor;

    pthread_create(&hilo_productor, NULL, productor, NULL);
    pthread_create(&hilo_consumidor, NULL, consumidor, NULL);

    pthread_join(hilo_productor, NULL);
    pthread_join(hilo_consumidor, NULL);

    sem_destroy(&vacios);
    sem_destroy(&llenos);

    return 0;
}
