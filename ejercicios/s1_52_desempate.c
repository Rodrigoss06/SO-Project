/*
 * s1_52_desempate.c
 * Ejercicio 5.2 — SJF con desempate (ráfaga -> AT -> PID).
 *
 * Dataset fijo diseñado para que, al ejecutar SJF no apropiativo, se
 * produzcan los dos primeros empates posibles según
 * common/planificador.c (simular_sjf_rr):
 *   - Empate de ráfaga con igual AT  -> se resuelve por PID.
 *   - Empate de ráfaga con distinto AT -> se resuelve por AT (menor AT).
 *
 * Imprime el dataset, el resultado de SJF (tabla + Gantt) y, debajo,
 * la traza explicando con qué regla se resolvió cada decisión.
 *
 * Compilar: make ejercicios
 *   (equivalente a: gcc -Wall -Wextra -std=c11 -Icommon
 *    ejercicios/s1_52_desempate.c common/proceso.c common/gantt.c
 *    common/planificador.c -o bin/s1_52_desempate)
 */

#include <stdio.h>

#include "gantt.h"
#include "planificador.h"
#include "proceso.h"

#define N_PROCESOS 6

int main(void) {
    /*           PID  AT  BT */
    ProcesoRR procesos[N_PROCESOS] = {
        {1, 0, 6, 6, 0, -1, 0, 0, 0},
        {2, 0, 2, 2, 0, -1, 0, 0, 0},
        {3, 0, 2, 2, 0, -1, 0, 0, 0},
        {4, 1, 2, 2, 0, -1, 0, 0, 0},
        {5, 3, 1, 1, 0, -1, 0, 0, 0},
        {6, 1, 4, 4, 0, -1, 0, 0, 0},
    };

    printf("=== Dataset ===\n\n");
    printf("%3s %3s %3s\n", "PID", "AT", "BT");
    for (int i = 0; i < N_PROCESOS; i++) {
        printf("%3d %3d %3d\n", procesos[i].pid, procesos[i].llegada, procesos[i].rafaga);
    }

    Gantt gantt;
    simular_sjf_rr(procesos, N_PROCESOS, &gantt);

    printf("\n=== SJF (no apropiativo) ===\n\n");
    imprimir_tabla_rr(procesos, N_PROCESOS, 0);
    printf("\nDiagrama de Gantt:\n");
    gantt_imprimir(&gantt);

    printf("\n=== Traza de desempates ===\n\n");
    printf("t=0 : disponibles P1(BT6,AT0), P2(BT2,AT0), P3(BT2,AT0).\n");
    printf("      Empate de menor BT entre P2 y P3 (ambos BT=2, AT=0)\n");
    printf("      -> se desempata por PID -> elige P2.\n\n");
    printf("t=2 : disponibles P1(BT6,AT0), P3(BT2,AT0), P4(BT2,AT1), P6(BT4,AT1).\n");
    printf("      Empate de menor BT entre P3 y P4 (ambos BT=2, AT distinto)\n");
    printf("      -> se desempata por AT (menor AT) -> elige P3 (AT=0).\n\n");
    printf("t=4 : disponibles P1(BT6), P4(BT2), P6(BT4), P5(BT1).\n");
    printf("      Menor BT sin empate -> elige P5 (BT=1).\n\n");
    printf("t=5 : disponibles P1(BT6), P4(BT2), P6(BT4).\n");
    printf("      Menor BT sin empate -> elige P4 (BT=2).\n\n");
    printf("t=7 : disponibles P1(BT6), P6(BT4).\n");
    printf("      Menor BT sin empate -> elige P6 (BT=4).\n\n");
    printf("t=11: solo queda P1 -> elige P1 (BT=6).\n");

    return 0;
}
