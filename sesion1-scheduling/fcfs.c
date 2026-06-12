/*
 * fcfs.c
 * Sesión 1 — Actividad A1: planificación FCFS (First Come First Served).
 *
 * Pide N procesos (AT, BT), los ordena por tiempo de llegada (desempate
 * por PID) y los ejecuta en ese orden sin apropiación. Calcula las
 * métricas (TAT, WT, RT), imprime la tabla con promedios y el diagrama
 * de Gantt.
 *
 * Compilar: make s1
 *   (equivalente a: gcc -Wall -Wextra -std=c11 -Icommon
 *    sesion1-scheduling/fcfs.c common/proceso.c common/gantt.c -o bin/fcfs)
 */

#include <stdio.h>

#include "gantt.h"
#include "proceso.h"

int main(void) {
    Proceso procesos[MAX_PROCESOS];
    int n = leer_procesos(procesos, MAX_PROCESOS);

    ordenar_por_llegada(procesos, n);

    Gantt gantt;
    gantt_iniciar(&gantt);

    int t = 0;
    for (int i = 0; i < n; i++) {
        Proceso *p = &procesos[i];

        if (t < p->llegada) {
            gantt_agregar(&gantt, 0, t, p->llegada); /* CPU inactiva */
            t = p->llegada;
        }

        p->inicio = t;
        p->fin = t + p->rafaga;
        calcular_metricas(p);

        gantt_agregar(&gantt, p->pid, p->inicio, p->fin);

        t = p->fin;
    }

    printf("\n=== FCFS (First Come First Served) ===\n\n");
    imprimir_tabla(procesos, n);

    printf("\nDiagrama de Gantt:\n");
    gantt_imprimir(&gantt);

    return 0;
}
