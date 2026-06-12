/*
 * s1_53_convoy.c
 * Ejercicio 5.3 — Efecto convoy en FCFS.
 *
 * El "efecto convoy" ocurre cuando un proceso largo (CPU-bound) entra a
 * ejecutar antes que varios procesos cortos que llegan casi al mismo
 * tiempo: en FCFS estos quedan atrapados detrás del proceso largo y su
 * tiempo de espera promedio se dispara, aunque su ráfaga sea mínima.
 *
 * Caso de prueba: 4 procesos llegan todos en t=0 -> P1 (BT=10, largo) y
 * P2, P3, P4 (BT=1, cortos). En FCFS, P1 se ejecuta primero (orden por
 * PID al haber empate de AT) y los 3 procesos cortos esperan detrás de
 * él. En SJF, los cortos se ejecutan primero y la espera promedio cae
 * drásticamente, evidenciando cómo SJF evita el efecto convoy.
 *
 * Compilar: make ejercicios
 *   (equivalente a: gcc -Wall -Wextra -std=c11 -Icommon
 *    ejercicios/s1_53_convoy.c common/proceso.c common/gantt.c
 *    common/planificador.c -o bin/s1_53_convoy)
 */

#include <stdio.h>

#include "gantt.h"
#include "planificador.h"
#include "proceso.h"

#define N_PROCESOS 4

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

static double espera_promedio(const ProcesoRR procesos[], int n) {
    double suma = 0.0;

    for (int i = 0; i < n; i++) {
        suma += procesos[i].espera;
    }
    return (n > 0) ? suma / n : 0.0;
}

int main(void) {
    /*           PID  AT  BT  restante prioridad inicio fin espera retorno */
    ProcesoRR base[N_PROCESOS] = {
        {1, 0, 10, 10, 0, -1, 0, 0, 0}, /* proceso largo */
        {2, 0,  1,  1, 0, -1, 0, 0, 0}, /* corto */
        {3, 0,  1,  1, 0, -1, 0, 0, 0}, /* corto */
        {4, 0,  1,  1, 0, -1, 0, 0, 0}, /* corto */
    };

    printf("=== Dataset (los 4 procesos llegan en t=0) ===\n\n");
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

    printf("\n=== Efecto convoy ===\n\n");
    printf("FCFS: P1 (BT=10) entra primero por orden de PID (empate de AT=0)\n");
    printf("y P2, P3, P4 (BT=1) quedan en \"convoy\" detrás de él.\n");
    printf("Espera promedio FCFS = %.2f\n\n", espera_promedio(fcfs, N_PROCESOS));
    printf("SJF: los procesos cortos se ejecutan primero; P1 pasa al final.\n");
    printf("Espera promedio SJF  = %.2f\n\n", espera_promedio(sjf, N_PROCESOS));
    printf("Conclusión: bajo FCFS, procesos cortos pueden esperar mucho tiempo\n");
    printf("detrás de uno largo (convoy). SJF reduce drásticamente la espera\n");
    printf("promedio al priorizar las ráfagas cortas.\n");

    return 0;
}
