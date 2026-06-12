/*
 * sjf.c
 * Sesión 1 — Actividad A2: planificación SJF no apropiativo
 * (Shortest Job First).
 *
 * Pide N procesos (AT, BT). En cada paso elige, entre los procesos ya
 * llegados (AT <= t) y no ejecutados, el de menor ráfaga (desempate por
 * AT y luego por PID). Si no hay procesos disponibles, avanza `t` hasta
 * la próxima llegada (CPU inactiva). Calcula métricas (TAT, WT, RT),
 * imprime la tabla con promedios y el diagrama de Gantt.
 *
 * Compilar: make s1
 *   (equivalente a: gcc -Wall -Wextra -std=c11 -Icommon
 *    sesion1-scheduling/sjf.c common/proceso.c common/gantt.c -o bin/sjf)
 */

#include <limits.h>
#include <stdio.h>

#include "gantt.h"
#include "proceso.h"

int main(void) {
    Proceso procesos[MAX_PROCESOS];
    int n = leer_procesos(procesos, MAX_PROCESOS);

    int hecho[MAX_PROCESOS] = {0};

    Gantt gantt;
    gantt_iniciar(&gantt);

    int t = 0;
    int completados = 0;

    while (completados < n) {
        int sel = -1;

        for (int i = 0; i < n; i++) {
            if (hecho[i] || procesos[i].llegada > t) {
                continue;
            }
            if (sel == -1
                || procesos[i].rafaga < procesos[sel].rafaga
                || (procesos[i].rafaga == procesos[sel].rafaga
                    && procesos[i].llegada < procesos[sel].llegada)
                || (procesos[i].rafaga == procesos[sel].rafaga
                    && procesos[i].llegada == procesos[sel].llegada
                    && procesos[i].pid < procesos[sel].pid)) {
                sel = i;
            }
        }

        if (sel == -1) {
            /* No hay procesos disponibles: CPU inactiva hasta la
             * próxima llegada. */
            int proxima_llegada = INT_MAX;
            for (int i = 0; i < n; i++) {
                if (!hecho[i] && procesos[i].llegada < proxima_llegada) {
                    proxima_llegada = procesos[i].llegada;
                }
            }
            gantt_agregar(&gantt, 0, t, proxima_llegada);
            t = proxima_llegada;
            continue;
        }

        Proceso *p = &procesos[sel];

        p->inicio = t;
        p->fin = t + p->rafaga;
        calcular_metricas(p);

        gantt_agregar(&gantt, p->pid, p->inicio, p->fin);

        t = p->fin;
        hecho[sel] = 1;
        completados++;
    }

    printf("\n=== SJF (Shortest Job First, no apropiativo) ===\n\n");
    imprimir_tabla(procesos, n);

    printf("\nDiagrama de Gantt:\n");
    gantt_imprimir(&gantt);

    return 0;
}
