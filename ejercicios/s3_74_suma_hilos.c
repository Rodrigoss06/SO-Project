/*
 * s3_74_suma_hilos.c
 * Ejercicio 7.4 — Suma paralela de un arreglo: de fork() a hilos.
 *
 * Con fork(), cada proceso hijo obtiene una COPIA del arreglo (memoria
 * separada): para combinar las sumas parciales del padre y los hijos se
 * necesitaría IPC explícito (pipes, memoria compartida, etc.). Con
 * pthreads, todos los hilos comparten el mismo espacio de direcciones,
 * así que cada hilo puede escribir su suma parcial directamente en una
 * posición de un arreglo compartido `sumas_parciales[]` (una posición
 * por hilo, sin que se pisen entre sí) y el hilo principal solo necesita
 * sumarlas tras el `pthread_join`. Por eso esta versión con hilos es más
 * simple que su equivalente con fork + IPC, y además evita la copia del
 * arreglo de entrada.
 *
 * El arreglo se divide en NUM_HILOS bloques contiguos; cada hilo recibe
 * por puntero (a memoria propia, nunca `&i` del bucle) el rango
 * [inicio, fin) que debe sumar. El resultado se compara contra la suma
 * secuencial y se mide el tiempo de cada enfoque con clock_gettime.
 *
 * Compilar: make ejercicios
 *   (equivalente a: gcc -Wall -Wextra -std=c11 -pthread
 *    ejercicios/s3_74_suma_hilos.c -o bin/s3_74_suma_hilos)
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TAM_ARREGLO 1000000
#define NUM_HILOS 4

static long arreglo[TAM_ARREGLO];

typedef struct {
    int inicio; /* inclusive */
    int fin;    /* exclusivo */
    long suma;
} BloqueSuma;

static void *sumar_bloque(void *arg) {
    BloqueSuma *b = (BloqueSuma *) arg;
    long suma = 0;

    for (int i = b->inicio; i < b->fin; i++) {
        suma += arreglo[i];
    }
    b->suma = suma;

    return NULL;
}

static double diferencia_seg(struct timespec inicio, struct timespec fin) {
    return (double) (fin.tv_sec - inicio.tv_sec)
         + (double) (fin.tv_nsec - inicio.tv_nsec) / 1e9;
}

int main(void) {
    for (int i = 0; i < TAM_ARREGLO; i++) {
        arreglo[i] = i + 1;
    }

    struct timespec t0, t1;

    /* --- Suma secuencial --- */
    clock_gettime(CLOCK_MONOTONIC, &t0);
    long suma_secuencial = 0;
    for (int i = 0; i < TAM_ARREGLO; i++) {
        suma_secuencial += arreglo[i];
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double tiempo_secuencial = diferencia_seg(t0, t1);

    /* --- Suma paralela con hilos --- */
    pthread_t hilos[NUM_HILOS];
    BloqueSuma bloques[NUM_HILOS];
    int tam_bloque = TAM_ARREGLO / NUM_HILOS;

    clock_gettime(CLOCK_MONOTONIC, &t0);

    for (int i = 0; i < NUM_HILOS; i++) {
        bloques[i].inicio = i * tam_bloque;
        bloques[i].fin = (i == NUM_HILOS - 1) ? TAM_ARREGLO : (i + 1) * tam_bloque;
        bloques[i].suma = 0;
        pthread_create(&hilos[i], NULL, sumar_bloque, &bloques[i]);
    }

    long suma_paralela = 0;
    for (int i = 0; i < NUM_HILOS; i++) {
        pthread_join(hilos[i], NULL);
        suma_paralela += bloques[i].suma;
    }

    clock_gettime(CLOCK_MONOTONIC, &t1);
    double tiempo_paralelo = diferencia_seg(t0, t1);

    printf("=== Suma paralela de un arreglo de %d elementos (%d hilos) ===\n\n",
           TAM_ARREGLO, NUM_HILOS);

    for (int i = 0; i < NUM_HILOS; i++) {
        printf("Hilo %d -> rango [%d, %d) -> suma parcial = %ld\n",
               i, bloques[i].inicio, bloques[i].fin, bloques[i].suma);
    }

    printf("\n%-12s %15s %12s\n", "Modo", "Suma", "Tiempo (s)");
    printf("%-12s %15ld %12.6f\n", "secuencial", suma_secuencial, tiempo_secuencial);
    printf("%-12s %15ld %12.6f\n", "con hilos", suma_paralela, tiempo_paralelo);

    if (suma_secuencial == suma_paralela) {
        printf("\nLas sumas coinciden: %ld\n", suma_secuencial);
    } else {
        printf("\nERROR: las sumas NO coinciden (%ld != %ld)\n",
               suma_secuencial, suma_paralela);
    }

    printf("\nfork vs hilos: con fork() cada proceso hijo trabajaría sobre\n");
    printf("una COPIA del arreglo y su suma parcial viviría en su propia\n");
    printf("memoria, por lo que el padre necesitaría IPC (pipe, memoria\n");
    printf("compartida) para recolectar los resultados. Con hilos, todos\n");
    printf("comparten el arreglo y escriben su suma parcial en\n");
    printf("`bloques[i].suma`, así que combinarlas es solo un bucle tras\n");
    printf("los `pthread_join`, sin IPC ni copias.\n");

    return 0;
}
