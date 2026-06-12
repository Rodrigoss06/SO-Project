/*
 * s1_51_aleatorio.c
 * Ejercicio 5.1 — FCFS vs SJF con 6 procesos aleatorios.
 *
 * Genera 6 procesos con AT (0..9) y BT (1..8) aleatorios (srand con
 * semilla; admite --seed N) y ejecuta FCFS y SJF (no apropiativo) sobre
 * el mismo dataset. Imprime ambas tablas con su diagrama de Gantt y, al
 * final, una tabla resumen comparando la espera y el retorno promedio.
 *
 * Uso: ./s1_51_aleatorio [--seed N]
 *
 * Compilar: make ejercicios
 *   (equivalente a: gcc -Wall -Wextra -std=c11 -Icommon
 *    ejercicios/s1_51_aleatorio.c common/proceso.c common/gantt.c
 *    common/planificador.c -o bin/s1_51_aleatorio)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "gantt.h"
#include "planificador.h"
#include "proceso.h"

#define N_PROCESOS 6

/* Reinicia los campos que cada simulación recalcula. */
static void preparar_copia(const ProcesoRR origen[], ProcesoRR copia[], int n) {
    for (int i = 0; i < n; i++) {
        copia[i].pid = origen[i].pid;
        copia[i].llegada = origen[i].llegada;
        copia[i].rafaga = origen[i].rafaga;
        copia[i].prioridad = origen[i].prioridad;
        copia[i].restante = origen[i].rafaga;
        copia[i].inicio = -1;
        copia[i].fin = 0;
        copia[i].espera = 0;
        copia[i].retorno = 0;
    }
}

static void promedios(const ProcesoRR procesos[], int n, double *espera, double *retorno) {
    double suma_espera = 0.0;
    double suma_retorno = 0.0;

    for (int i = 0; i < n; i++) {
        suma_espera += procesos[i].espera;
        suma_retorno += procesos[i].retorno;
    }

    *espera = (n > 0) ? suma_espera / n : 0.0;
    *retorno = (n > 0) ? suma_retorno / n : 0.0;
}

int main(int argc, char *argv[]) {
    unsigned int semilla = (unsigned int) time(NULL);

    if (argc == 3 && strcmp(argv[1], "--seed") == 0) {
        semilla = (unsigned int) atoi(argv[2]);
    }
    srand(semilla);

    ProcesoRR base[N_PROCESOS];

    for (int i = 0; i < N_PROCESOS; i++) {
        base[i].pid = i + 1;
        base[i].llegada = rand() % 10;
        base[i].rafaga = 1 + rand() % 8;
        base[i].prioridad = 0;
    }

    printf("=== Procesos generados (semilla %u) ===\n\n", semilla);
    printf("%3s %3s %3s\n", "PID", "AT", "BT");
    for (int i = 0; i < N_PROCESOS; i++) {
        printf("%3d %3d %3d\n", base[i].pid, base[i].llegada, base[i].rafaga);
    }

    ProcesoRR fcfs[N_PROCESOS];
    ProcesoRR sjf[N_PROCESOS];
    Gantt g_fcfs, g_sjf;

    preparar_copia(base, fcfs, N_PROCESOS);
    simular_fcfs_rr(fcfs, N_PROCESOS, &g_fcfs);

    preparar_copia(base, sjf, N_PROCESOS);
    simular_sjf_rr(sjf, N_PROCESOS, &g_sjf);

    printf("\n=== FCFS ===\n\n");
    imprimir_tabla_rr(fcfs, N_PROCESOS, 0);
    printf("\nDiagrama de Gantt:\n");
    gantt_imprimir(&g_fcfs);

    printf("\n=== SJF (no apropiativo) ===\n\n");
    imprimir_tabla_rr(sjf, N_PROCESOS, 0);
    printf("\nDiagrama de Gantt:\n");
    gantt_imprimir(&g_sjf);

    double esp_fcfs, ret_fcfs, esp_sjf, ret_sjf;
    promedios(fcfs, N_PROCESOS, &esp_fcfs, &ret_fcfs);
    promedios(sjf, N_PROCESOS, &esp_sjf, &ret_sjf);

    printf("\n=== Tabla resumen ===\n\n");
    printf("%-12s %12s %14s\n", "Algoritmo", "Espera prom.", "Retorno prom.");
    printf("%-12s %12.2f %14.2f\n", "FCFS", esp_fcfs, ret_fcfs);
    printf("%-12s %12.2f %14.2f\n", "SJF", esp_sjf, ret_sjf);

    return 0;
}
