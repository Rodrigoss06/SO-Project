/*
 * simulador.c
 * Sesión 4 — Proyecto integrador: escenario Banco/Caja.
 *
 * M ventanillas modeladas con un semáforo contador (sem_t recurso,
 * inicializado en `capacidad`). N clientes (hilos) llegan en instantes
 * aleatorios y requieren un tiempo de atención (ráfaga) aleatorio.
 * Planificación FCFS por defecto; con --prioridad, los clientes marcados
 * "adulto mayor" pasan primero (dentro de los que esperan turno),
 * manteniendo FCFS entre clientes de igual prioridad.
 *
 * Cada cliente: sem_wait(&recurso) -> usa el recurso (sleep = ráfaga) ->
 * sem_post(&recurso). El contador `ocupado` y la escritura del log
 * (consola + log_simulacion.txt) están protegidos por mutex_estado.
 *
 * Uso: ./simulador [n_clientes] [capacidad] [--prioridad] [--seed N]
 *   n_clientes (por defecto 6), capacidad = ventanillas (por defecto 3)
 *
 * Compilar: make s4
 *   (equivalente a: gcc -Wall -Wextra -std=c11 -Icommon
 *    sesion4-integrador/simulador.c common/gantt.c common/planificador.c
 *    common/proceso.c -o bin/simulador -pthread)
 */

#include <pthread.h>
#include <semaphore.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define MAX_CLIENTES 100

typedef struct {
    int id;
    int llegada;
    int rafaga;
    int adulto_mayor;
} Cliente;

static Cliente clientes[MAX_CLIENTES];
static int n_clientes;
static int capacidad;
static int modo_prioridad = 0;

static sem_t recurso;
static pthread_mutex_t mutex_estado = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cv_turno = PTHREAD_COND_INITIALIZER;

static int cola[MAX_CLIENTES];
static int tam_cola = 0;
static int ocupado = 0;

static double suma_espera = 0.0;
static struct timeval t0;
static FILE *log_archivo;

/* Segundos transcurridos desde t0. */
static double tiempo_transcurrido(void) {
    struct timeval ahora;

    gettimeofday(&ahora, NULL);
    return (ahora.tv_sec - t0.tv_sec) + (ahora.tv_usec - t0.tv_usec) / 1e6;
}

/* Instante lógico (segundos enteros redondeados) para los logs. */
static int t_actual(void) {
    return (int) (tiempo_transcurrido() + 0.5);
}

/* Escribe un evento "[t=T] ..." en consola y en el log. El llamador
 * debe tener tomado mutex_estado. */
static void registrar(int t, const char *formato, ...) {
    va_list args;

    printf("[t=%d] ", t);
    va_start(args, formato);
    vprintf(formato, args);
    va_end(args);
    printf("\n");

    fprintf(log_archivo, "[t=%d] ", t);
    va_start(args, formato);
    vfprintf(log_archivo, formato, args);
    va_end(args);
    fprintf(log_archivo, "\n");
}

/* <0 si el cliente a debe atenderse antes que el cliente b. */
static int comparar_prioridad(int a, int b) {
    const Cliente *ca = &clientes[a];
    const Cliente *cb = &clientes[b];

    if (modo_prioridad && ca->adulto_mayor != cb->adulto_mayor) {
        return cb->adulto_mayor - ca->adulto_mayor; /* adulto mayor primero */
    }
    if (ca->llegada != cb->llegada) {
        return ca->llegada - cb->llegada;
    }
    return ca->id - cb->id;
}

/* true si ningún otro cliente en cola debe atenderse antes que idx. */
static int es_su_turno(int idx) {
    for (int k = 0; k < tam_cola; k++) {
        if (cola[k] != idx && comparar_prioridad(cola[k], idx) < 0) {
            return 0;
        }
    }
    return 1;
}

/* true si idx puede pasar ya: es su turno por prioridad y hay un
 * lugar libre en el recurso. */
static int puede_pasar(int idx) {
    return es_su_turno(idx) && ocupado < capacidad;
}

static void *atender_cliente(void *arg) {
    int idx = *(int *) arg;
    Cliente *c = &clientes[idx];

    sleep((unsigned) c->llegada);

    pthread_mutex_lock(&mutex_estado);

    int t_llegada = t_actual();
    cola[tam_cola++] = idx;

    if (puede_pasar(idx)) {
        registrar(t_llegada, "Cliente %d LLEGA y solicita recurso", c->id);
    } else {
        registrar(t_llegada, "Cliente %d LLEGA y solicita recurso (en espera)", c->id);
    }

    while (!puede_pasar(idx)) {
        pthread_cond_wait(&cv_turno, &mutex_estado);
    }
    for (int k = 0; k < tam_cola; k++) {
        if (cola[k] == idx) {
            for (int j = k; j < tam_cola - 1; j++) {
                cola[j] = cola[j + 1];
            }
            tam_cola--;
            break;
        }
    }

    /* ocupado < capacidad ya verificado: sem_wait no bloquea. */
    sem_wait(&recurso);
    ocupado++;
    int t_ingreso = t_actual();
    suma_espera += (t_ingreso - t_llegada);
    registrar(t_ingreso, "Cliente %d INGRESA (recurso ocupado: %d/%d)", c->id, ocupado, capacidad);

    pthread_mutex_unlock(&mutex_estado);

    sleep((unsigned) c->rafaga);

    pthread_mutex_lock(&mutex_estado);
    ocupado--;
    sem_post(&recurso);
    registrar(t_actual(), "Cliente %d SALE (recurso ocupado: %d/%d)", c->id, ocupado, capacidad);
    pthread_cond_broadcast(&cv_turno);
    pthread_mutex_unlock(&mutex_estado);

    return NULL;
}

int main(int argc, char *argv[]) {
    n_clientes = 6;
    capacidad = 3;
    unsigned int semilla = (unsigned int) time(NULL);

    int argi = 1;

    if (argi < argc && argv[argi][0] != '-') {
        n_clientes = atoi(argv[argi++]);
    }
    if (argi < argc && argv[argi][0] != '-') {
        capacidad = atoi(argv[argi++]);
    }
    for (; argi < argc; argi++) {
        if (strcmp(argv[argi], "--prioridad") == 0) {
            modo_prioridad = 1;
        } else if (strcmp(argv[argi], "--seed") == 0 && argi + 1 < argc) {
            semilla = (unsigned int) atoi(argv[++argi]);
        }
    }

    if (n_clientes < 1) {
        n_clientes = 1;
    }
    if (n_clientes > MAX_CLIENTES) {
        n_clientes = MAX_CLIENTES;
    }
    if (capacidad < 1) {
        capacidad = 1;
    }

    srand(semilla);

    for (int i = 0; i < n_clientes; i++) {
        clientes[i].id = i + 1;
        clientes[i].llegada = rand() % n_clientes;
        clientes[i].rafaga = 1 + rand() % 3;
        clientes[i].adulto_mayor = (rand() % 100) < 30;
    }

    log_archivo = fopen("log_simulacion.txt", "w");
    if (log_archivo == NULL) {
        perror("fopen");
        return 1;
    }

    printf("=== Simulación Banco/Caja ===\n");
    printf("Clientes: %d | Ventanillas: %d | Modo: %s | Semilla: %u\n",
           n_clientes, capacidad,
           modo_prioridad ? "FCFS + prioridad adulto mayor" : "FCFS",
           semilla);
    fprintf(log_archivo, "=== Simulación Banco/Caja ===\n");
    fprintf(log_archivo, "Clientes: %d | Ventanillas: %d | Modo: %s | Semilla: %u\n",
            n_clientes, capacidad,
            modo_prioridad ? "FCFS + prioridad adulto mayor" : "FCFS",
            semilla);

    for (int i = 0; i < n_clientes; i++) {
        printf("  Cliente %d: llegada=%d  atencion=%d%s\n",
               clientes[i].id, clientes[i].llegada, clientes[i].rafaga,
               clientes[i].adulto_mayor ? "  (adulto mayor)" : "");
        fprintf(log_archivo, "  Cliente %d: llegada=%d  atencion=%d%s\n",
                clientes[i].id, clientes[i].llegada, clientes[i].rafaga,
                clientes[i].adulto_mayor ? "  (adulto mayor)" : "");
    }
    printf("\n");
    fprintf(log_archivo, "\n");

    sem_init(&recurso, 0, (unsigned) capacidad);
    gettimeofday(&t0, NULL);

    pthread_t hilos[MAX_CLIENTES];
    int ids[MAX_CLIENTES];

    for (int i = 0; i < n_clientes; i++) {
        ids[i] = i;
        pthread_create(&hilos[i], NULL, atender_cliente, &ids[i]);
    }
    for (int i = 0; i < n_clientes; i++) {
        pthread_join(hilos[i], NULL);
    }

    double duracion_total = tiempo_transcurrido();
    double suma_rafagas = 0.0;

    for (int i = 0; i < n_clientes; i++) {
        suma_rafagas += clientes[i].rafaga;
    }

    double espera_promedio = suma_espera / n_clientes;
    double uso_pct = (suma_rafagas / (capacidad * duracion_total)) * 100.0;

    printf("\n=== Métricas finales ===\n");
    printf("Tiempo total de simulación: %.2f s\n", duracion_total);
    printf("Espera promedio: %.2f s\n", espera_promedio);
    printf("Uso del recurso: %.2f%%\n", uso_pct);

    fprintf(log_archivo, "\n=== Métricas finales ===\n");
    fprintf(log_archivo, "Tiempo total de simulación: %.2f s\n", duracion_total);
    fprintf(log_archivo, "Espera promedio: %.2f s\n", espera_promedio);
    fprintf(log_archivo, "Uso del recurso: %.2f%%\n", uso_pct);

    fclose(log_archivo);
    sem_destroy(&recurso);

    return 0;
}
