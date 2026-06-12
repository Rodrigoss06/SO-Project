/*
 * s1_54_respuesta.c
 * Ejercicio 5.4 — Tiempo de respuesta (RT) para FCFS y SJF.
 *
 * RT (response time) = inicio - AT: cuánto tarda un proceso en obtener
 * la CPU por primera vez. common/proceso.h no agrega un campo RT a
 * ProcesoRR porque, para algoritmos no apropiativos, `inicio` ya es el
 * único instante en que el proceso usa la CPU, así que
 * RT = inicio - AT = (CT - BT) - AT = (CT - AT) - BT = TAT - BT = WT.
 * Es decir: en FCFS y SJF (no apropiativos), RT y WT son siempre
 * iguales. (En RR, al ser apropiativo, `inicio` es solo el primer
 * despacho y RT <= WT en general.)
 *
 * Este programa corre FCFS y SJF sobre el mismo dataset, imprime una
 * tabla con la columna RT añadida y verifica RT == WT para cada
 * proceso.
 *
 * Compilar: make ejercicios
 *   (equivalente a: gcc -Wall -Wextra -std=c11 -Icommon
 *    ejercicios/s1_54_respuesta.c common/proceso.c common/gantt.c
 *    common/planificador.c -o bin/s1_54_respuesta)
 */

#include <stdio.h>

#include "gantt.h"
#include "planificador.h"
#include "proceso.h"

#define N_PROCESOS 5

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

/* Imprime PID/AT/BT/Inicio/Fin/Espera/Retorno/RT y verifica RT == WT. */
static void imprimir_con_rt(const ProcesoRR procesos[], int n) {
    printf("%3s %3s %3s %6s %3s %6s %7s %3s\n",
           "PID", "AT", "BT", "Inicio", "Fin", "Espera", "Retorno", "RT");

    for (int i = 0; i < n; i++) {
        const ProcesoRR *p = &procesos[i];
        int rt = p->inicio - p->llegada;

        printf("%3d %3d %3d %6d %3d %6d %7d %3d", p->pid, p->llegada,
               p->rafaga, p->inicio, p->fin, p->espera, p->retorno, rt);

        if (rt != p->espera) {
            printf("   <-- RT != WT (inesperado)");
        }
        printf("\n");
    }
}

int main(void) {
    /*           PID  AT  BT  restante prioridad inicio fin espera retorno */
    ProcesoRR base[N_PROCESOS] = {
        {1, 0, 4, 4, 0, -1, 0, 0, 0},
        {2, 1, 3, 3, 0, -1, 0, 0, 0},
        {3, 2, 1, 1, 0, -1, 0, 0, 0},
        {4, 3, 2, 2, 0, -1, 0, 0, 0},
        {5, 4, 5, 5, 0, -1, 0, 0, 0},
    };

    printf("=== Dataset ===\n\n");
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

    printf("\n=== FCFS (con columna RT) ===\n\n");
    imprimir_con_rt(fcfs, N_PROCESOS);
    printf("\nDiagrama de Gantt:\n");
    gantt_imprimir(&g_fcfs);

    printf("\n=== SJF no apropiativo (con columna RT) ===\n\n");
    imprimir_con_rt(sjf, N_PROCESOS);
    printf("\nDiagrama de Gantt:\n");
    gantt_imprimir(&g_sjf);

    printf("\nConclusión: en ambos algoritmos RT == WT para todos los\n");
    printf("procesos, porque al ser no apropiativos cada proceso usa la\n");
    printf("CPU en un único tramo continuo (inicio..fin).\n");

    return 0;
}
