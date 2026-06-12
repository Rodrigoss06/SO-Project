/*
 * gantt.c
 * Implementación del diagrama de Gantt en texto.
 */

#include <stdio.h>

#include "gantt.h"

void gantt_iniciar(Gantt *g) {
    g->n = 0;
}

void gantt_agregar(Gantt *g, int pid, int t_inicio, int t_fin) {
    if (t_inicio == t_fin) {
        return;
    }

    if (g->n > 0) {
        EventoGantt *ultimo = &g->eventos[g->n - 1];
        if (ultimo->pid == pid && ultimo->fin == t_inicio) {
            ultimo->fin = t_fin;
            return;
        }
    }

    if (g->n < GANTT_MAX_EVENTOS) {
        g->eventos[g->n].pid = pid;
        g->eventos[g->n].inicio = t_inicio;
        g->eventos[g->n].fin = t_fin;
        g->n++;
    }
}

void gantt_imprimir(const Gantt *g) {
    if (g->n == 0) {
        return;
    }

    int anchos[GANTT_MAX_EVENTOS];

    for (int i = 0; i < g->n; i++) {
        int pid = g->eventos[i].pid;
        int largo_etiqueta;

        if (pid > 0) {
            int tmp = pid;
            largo_etiqueta = 1; /* 'P' */
            do {
                largo_etiqueta++;
                tmp /= 10;
            } while (tmp != 0);

            printf("| P%d ", pid);
        } else {
            largo_etiqueta = 4; /* "IDLE" */
            printf("| IDLE ");
        }

        anchos[i] = largo_etiqueta + 3; /* "| " + etiqueta + " " */
    }
    printf("|\n");

    for (int i = 0; i < g->n; i++) {
        printf("%-*d", anchos[i], g->eventos[i].inicio);
    }
    printf("%d\n", g->eventos[g->n - 1].fin);
}
