/*
 * comparativa.c
 * Sesión 2 — Actividad A3: comparación de FCFS, SJF, Round Robin
 * (q=2 y q=4) y Prioridades sobre el mismo dataset (AT, BT,
 * prioridad).
 *
 * Para cada algoritmo imprime la tabla de resultados y el diagrama de
 * Gantt, y al final arma la tabla comparativa de espera promedio,
 * retorno promedio y número de cambios de contexto (solo aplica a
 * Round Robin; en el resto se muestra "-").
 *
 * Compilar: make s2
 *   (equivalente a: gcc -Wall -Wextra -std=c11 -Icommon
 *    sesion2-rr-prioridades/comparativa.c common/proceso.c
 *    common/gantt.c common/planificador.c -o bin/comparativa)
 */

#include <stdio.h>

#include "gantt.h"
#include "planificador.h"
#include "proceso.h"

/* Copia el dataset base a una copia de trabajo, reiniciando los
 * campos que cada simulación va a recalcular. */
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

static void mostrar_resultado(const char *titulo, const ProcesoRR procesos[], int n,
                               const Gantt *gantt) {
    printf("\n=== %s ===\n\n", titulo);
    imprimir_tabla_rr(procesos, n, 1);
    printf("\nDiagrama de Gantt:\n");
    gantt_imprimir(gantt);
}

int main(void) {
    ProcesoRR base[MAX_PROCESOS];
    int n = leer_procesos_rr(base, MAX_PROCESOS, 1);

    ProcesoRR fcfs[MAX_PROCESOS];
    ProcesoRR sjf[MAX_PROCESOS];
    ProcesoRR rr2[MAX_PROCESOS];
    ProcesoRR rr4[MAX_PROCESOS];
    ProcesoRR prio[MAX_PROCESOS];

    Gantt g_fcfs, g_sjf, g_rr2, g_rr4, g_prio;
    int cc_rr2, cc_rr4;

    preparar_copia(base, fcfs, n);
    simular_fcfs_rr(fcfs, n, &g_fcfs);
    mostrar_resultado("FCFS", fcfs, n, &g_fcfs);

    preparar_copia(base, sjf, n);
    simular_sjf_rr(sjf, n, &g_sjf);
    mostrar_resultado("SJF (no apropiativo)", sjf, n, &g_sjf);

    preparar_copia(base, rr2, n);
    simular_rr(rr2, n, 2, &g_rr2, &cc_rr2);
    mostrar_resultado("Round Robin (quantum = 2)", rr2, n, &g_rr2);
    printf("\nCambios de contexto: %d\n", cc_rr2);

    preparar_copia(base, rr4, n);
    simular_rr(rr4, n, 4, &g_rr4, &cc_rr4);
    mostrar_resultado("Round Robin (quantum = 4)", rr4, n, &g_rr4);
    printf("\nCambios de contexto: %d\n", cc_rr4);

    preparar_copia(base, prio, n);
    simular_prioridades(prio, n, &g_prio);
    mostrar_resultado("Prioridades (no apropiativo)", prio, n, &g_prio);

    double esp_fcfs, ret_fcfs, esp_sjf, ret_sjf;
    double esp_rr2, ret_rr2, esp_rr4, ret_rr4, esp_prio, ret_prio;

    promedios(fcfs, n, &esp_fcfs, &ret_fcfs);
    promedios(sjf, n, &esp_sjf, &ret_sjf);
    promedios(rr2, n, &esp_rr2, &ret_rr2);
    promedios(rr4, n, &esp_rr4, &ret_rr4);
    promedios(prio, n, &esp_prio, &ret_prio);

    printf("\n=== Tabla comparativa ===\n\n");
    printf("%-12s %12s %14s %20s\n",
           "Algoritmo", "Espera prom.", "Retorno prom.", "Cambios contexto");
    printf("%-12s %12.2f %14.2f %20s\n", "FCFS", esp_fcfs, ret_fcfs, "-");
    printf("%-12s %12.2f %14.2f %20s\n", "SJF", esp_sjf, ret_sjf, "-");
    printf("%-12s %12.2f %14.2f %20d\n", "RR (q=2)", esp_rr2, ret_rr2, cc_rr2);
    printf("%-12s %12.2f %14.2f %20d\n", "RR (q=4)", esp_rr4, ret_rr4, cc_rr4);
    printf("%-12s %12.2f %14.2f %20s\n", "Prioridades", esp_prio, ret_prio, "-");

    return 0;
}
