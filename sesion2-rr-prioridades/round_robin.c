/*
 * round_robin.c
 * Sesión 2 — Actividad A1: planificación Round Robin (RR).
 *
 * Pide N procesos (AT, BT) y un quantum. Usa una cola circular FIFO:
 * en cada despacho ejecuta min(quantum, restante); si el proceso no
 * termina, primero se encolan los procesos recién llegados en ese
 * instante y luego se reinserta el proceso desalojado al final de la
 * cola (convención de reinserción). Calcula CT/WT/TAT, imprime la
 * tabla con promedios, el diagrama de Gantt y el número de cambios de
 * contexto.
 *
 * Compilar: make s2
 *   (equivalente a: gcc -Wall -Wextra -std=c11 -Icommon
 *    sesion2-rr-prioridades/round_robin.c common/proceso.c
 *    common/gantt.c common/planificador.c -o bin/round_robin)
 */

#include <stdio.h>

#include "gantt.h"
#include "planificador.h"
#include "proceso.h"

int main(void) {
    ProcesoRR procesos[MAX_PROCESOS];
    int n = leer_procesos_rr(procesos, MAX_PROCESOS, 0);

    int quantum;
    printf("Quantum: ");
    while (scanf("%d", &quantum) != 1 || quantum <= 0) {
        printf("Ingrese un quantum mayor que 0: ");
        while (getchar() != '\n') {
            /* descarta el resto de la línea inválida */
        }
    }

    Gantt gantt;
    int cambios_contexto;
    simular_rr(procesos, n, quantum, &gantt, &cambios_contexto);

    printf("\n=== Round Robin (quantum = %d) ===\n\n", quantum);
    imprimir_tabla_rr(procesos, n, 0);

    printf("\nDiagrama de Gantt:\n");
    gantt_imprimir(&gantt);

    printf("\nCambios de contexto: %d\n", cambios_contexto);

    return 0;
}
