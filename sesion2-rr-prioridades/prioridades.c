/*
 * prioridades.c
 * Sesión 2 — Actividad A2: planificación por Prioridades, no
 * apropiativo.
 *
 * Pide N procesos (AT, BT, prioridad 1..5; 1 = mayor). En cada paso
 * elige, entre los procesos ya llegados (AT <= t) y no ejecutados, el
 * de mayor prioridad (menor número); empate -> FCFS (menor AT y, si
 * persiste, menor PID). Si no hay disponibles, avanza `t` hasta la
 * próxima llegada (CPU inactiva). Calcula CT/WT/TAT, imprime la tabla
 * con promedios y el diagrama de Gantt.
 *
 * Compilar: make s2
 *   (equivalente a: gcc -Wall -Wextra -std=c11 -Icommon
 *    sesion2-rr-prioridades/prioridades.c common/proceso.c
 *    common/gantt.c common/planificador.c -o bin/prioridades)
 */

#include <stdio.h>

#include "gantt.h"
#include "planificador.h"
#include "proceso.h"

int main(void) {
    ProcesoRR procesos[MAX_PROCESOS];
    int n = leer_procesos_rr(procesos, MAX_PROCESOS, 1);

    Gantt gantt;
    simular_prioridades(procesos, n, &gantt);

    printf("\n=== Prioridades (no apropiativo) ===\n\n");
    imprimir_tabla_rr(procesos, n, 1);

    printf("\nDiagrama de Gantt:\n");
    gantt_imprimir(&gantt);

    return 0;
}
