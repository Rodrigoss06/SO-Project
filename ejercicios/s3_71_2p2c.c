/*
 * s3_71_2p2c.c
 * Ejercicio 7.1 — Productor-Consumidor con 2 productores y 2 consumidores.
 *
 * Extiende sesion3-hilos/productor_consumidor.c (1P/1C) a 2 productores y
 * 2 consumidores sobre un mismo buffer circular de tamaño 5. Cada
 * productor genera NUM_ITEMS valores y cada consumidor retira NUM_ITEMS
 * valores (2 * NUM_ITEMS producidos = 2 * NUM_ITEMS consumidos en total).
 *
 * Sincronización: sem_t vacios = TAM_BUFFER, llenos = 0, más un mutex
 * para el acceso al buffer e índices. Orden de operaciones (evita
 * deadlock mutex/semáforo invertidos):
 *   productor:  sem_wait(vacios) -> lock -> ... -> unlock -> sem_post(llenos)
 *   consumidor: sem_wait(llenos) -> lock -> ... -> unlock -> sem_post(vacios)
 *
 * Cada hilo recibe su identificador por puntero a memoria propia (un
 * arreglo `int ids[]`, nunca `&i` del bucle de creación).
 *
 * Uso: ./s3_71_2p2c [--seed N]
 *
 * Compilar: make ejercicios
 *   (equivalente a: gcc -Wall -Wextra -std=c11 -pthread
 *    ejercicios/s3_71_2p2c.c -o bin/s3_71_2p2c)
 */

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TAM_BUFFER 5
#define NUM_ITEMS 10 /* por cada productor / consumidor */
#define N_PRODUCTORES 2
#define N_CONSUMIDORES 2

static int buffer[TAM_BUFFER];
static int pos_in = 0;
static int pos_out = 0;

static sem_t vacios;
static sem_t llenos;
static pthread_mutex_t mutex_buffer = PTHREAD_MUTEX_INITIALIZER;

static void *productor(void *arg) {
    int id = *(int *) arg;

    for (int i = 0; i < NUM_ITEMS; i++) {
        int valor = 1 + rand() % 100;

        sem_wait(&vacios);
        pthread_mutex_lock(&mutex_buffer);

        buffer[pos_in] = valor;
        printf("Productor %d -> coloca %3d en posición %d\n", id, valor, pos_in);
        pos_in = (pos_in + 1) % TAM_BUFFER;

        pthread_mutex_unlock(&mutex_buffer);
        sem_post(&llenos);
    }

    return NULL;
}

static void *consumidor(void *arg) {
    int id = *(int *) arg;

    for (int i = 0; i < NUM_ITEMS; i++) {
        sem_wait(&llenos);
        pthread_mutex_lock(&mutex_buffer);

        int valor = buffer[pos_out];
        printf("Consumidor %d -> retira %3d de posición %d\n", id, valor, pos_out);
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

    pthread_t productores[N_PRODUCTORES];
    pthread_t consumidores[N_CONSUMIDORES];
    int ids_productores[N_PRODUCTORES];
    int ids_consumidores[N_CONSUMIDORES];

    printf("=== Productor-Consumidor 2P/2C (buffer de %d, %d ítems por hilo) ===\n\n",
           TAM_BUFFER, NUM_ITEMS);

    for (int i = 0; i < N_PRODUCTORES; i++) {
        ids_productores[i] = i + 1;
        pthread_create(&productores[i], NULL, productor, &ids_productores[i]);
    }
    for (int i = 0; i < N_CONSUMIDORES; i++) {
        ids_consumidores[i] = i + 1;
        pthread_create(&consumidores[i], NULL, consumidor, &ids_consumidores[i]);
    }

    for (int i = 0; i < N_PRODUCTORES; i++) {
        pthread_join(productores[i], NULL);
    }
    for (int i = 0; i < N_CONSUMIDORES; i++) {
        pthread_join(consumidores[i], NULL);
    }

    sem_destroy(&vacios);
    sem_destroy(&llenos);

    printf("\nTotal producido : %d\n", N_PRODUCTORES * NUM_ITEMS);
    printf("Total consumido : %d\n", N_CONSUMIDORES * NUM_ITEMS);

    return 0;
}
