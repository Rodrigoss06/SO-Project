/*
 * s2_62_rr_prio.c
 * Ejercicio 6.2 — Round Robin apropiativo con prioridades.
 *
 * Cada nivel de prioridad (1 = mayor .. 5 = menor) tiene su propia cola
 * FIFO. La simulación avanza de a 1 unidad de tiempo:
 *   1. Encola los procesos que llegan en `t`. Si llega uno con
 *      prioridad mayor (número menor) que el proceso en ejecución, este
 *      se desaloja (vuelve al final de la cola de su nivel) y se libera
 *      la CPU.
 *   2. Si la CPU está libre, despacha el proceso al frente de la cola de
 *      mayor prioridad no vacía y le asigna un quantum completo.
 *   3. Ejecuta una unidad. Si termina su ráfaga, se completa. Si se
 *      agota el quantum sin terminar, se desaloja (vuelve al final de su
 *      cola).
 *
 * Dataset fijo con quantum = 2, diseñado para mostrar: RR dentro de un
 * mismo nivel de prioridad (P1/P5, prioridad 2) y desalojo por llegada
 * de un proceso de mayor prioridad (P2, prioridad 1, llega en t=1 y
 * desaloja a P1).
 *
 * Compilar: make ejercicios
 *   (equivalente a: gcc -Wall -Wextra -std=c11 -Icommon
 *    ejercicios/s2_62_rr_prio.c common/proceso.c common/gantt.c
 *    -o bin/s2_62_rr_prio)
 */

#include <stdio.h>

#include "gantt.h"
#include "proceso.h"

#define N_PROCESOS 5
#define NIVELES 6 /* índices 1..5 usados (1 = mayor prioridad) */
#define CAP_COLA 256
#define QUANTUM 2

static int cola[NIVELES][CAP_COLA];
static int frente[NIVELES], fondo[NIVELES], tam[NIVELES];

static void encolar(int nivel, int idx) {
    cola[nivel][fondo[nivel]] = idx;
    fondo[nivel] = (fondo[nivel] + 1) % CAP_COLA;
    tam[nivel]++;
}

static int desencolar(int nivel) {
    int idx = cola[nivel][frente[nivel]];
    frente[nivel] = (frente[nivel] + 1) % CAP_COLA;
    tam[nivel]--;
    return idx;
}

int main(void) {
    /*           PID  AT  BT  restante prioridad inicio fin espera retorno */
    ProcesoRR procesos[N_PROCESOS] = {
        {1, 0, 5, 5, 2, -1, 0, 0, 0},
        {2, 1, 3, 3, 1, -1, 0, 0, 0},
        {3, 2, 4, 4, 3, -1, 0, 0, 0},
        {4, 3, 2, 2, 1, -1, 0, 0, 0},
        {5, 0, 3, 3, 2, -1, 0, 0, 0},
    };

    printf("=== Dataset (quantum = %d, prioridad 1 = mayor) ===\n\n", QUANTUM);
    printf("%3s %3s %3s %4s\n", "PID", "AT", "BT", "Prio");
    for (int i = 0; i < N_PROCESOS; i++) {
        printf("%3d %3d %3d %4d\n", procesos[i].pid, procesos[i].llegada,
               procesos[i].rafaga, procesos[i].prioridad);
    }

    int llego[N_PROCESOS] = {0};
    Gantt gantt;
    gantt_iniciar(&gantt);

    int actual = -1;
    int q_restante = 0;
    int ultimo_pid = 0;
    int cambios_contexto = 0;
    int completados = 0;
    int t = 0;

    while (completados < N_PROCESOS) {
        for (int i = 0; i < N_PROCESOS; i++) {
            if (!llego[i] && procesos[i].llegada == t) {
                llego[i] = 1;
                encolar(procesos[i].prioridad, i);

                if (actual != -1 && procesos[i].prioridad < procesos[actual].prioridad) {
                    encolar(procesos[actual].prioridad, actual);
                    actual = -1;
                }
            }
        }

        if (actual == -1) {
            for (int nivel = 1; nivel <= 5; nivel++) {
                if (tam[nivel] > 0) {
                    actual = desencolar(nivel);
                    q_restante = QUANTUM;

                    if (procesos[actual].inicio == -1) {
                        procesos[actual].inicio = t;
                    }
                    if (ultimo_pid != 0 && ultimo_pid != procesos[actual].pid) {
                        cambios_contexto++;
                    }
                    ultimo_pid = procesos[actual].pid;
                    break;
                }
            }
        }

        if (actual == -1) {
            gantt_agregar(&gantt, 0, t, t + 1); /* CPU inactiva */
            t++;
            continue;
        }

        gantt_agregar(&gantt, procesos[actual].pid, t, t + 1);
        procesos[actual].restante--;
        q_restante--;
        t++;

        if (procesos[actual].restante == 0) {
            procesos[actual].fin = t;
            calcular_metricas_rr(&procesos[actual]);
            completados++;
            actual = -1;
        } else if (q_restante == 0) {
            encolar(procesos[actual].prioridad, actual);
            actual = -1;
        }
    }

    printf("\n=== RR apropiativo con prioridades ===\n\n");
    imprimir_tabla_rr(procesos, N_PROCESOS, 1);

    printf("\nDiagrama de Gantt:\n");
    gantt_imprimir(&gantt);

    printf("\nCambios de contexto: %d\n", cambios_contexto);

    return 0;
}
