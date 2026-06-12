/*
 * proceso.c
 * Implementación de las utilidades de la struct Proceso: lectura
 * interactiva, ordenamiento por llegada, cálculo de métricas e
 * impresión de la tabla de resultados con promedios.
 */

#include <stdio.h>
#include <stdlib.h>

#include "proceso.h"

int leer_procesos(Proceso procesos[], int max) {
    int n;

    printf("¿Cuántos procesos desea ingresar? ");
    while (scanf("%d", &n) != 1 || n <= 0 || n > max) {
        printf("Ingrese un número entre 1 y %d: ", max);
        while (getchar() != '\n') {
            /* descarta el resto de la línea inválida */
        }
    }

    for (int i = 0; i < n; i++) {
        procesos[i].pid = i + 1;

        printf("Proceso P%d - tiempo de llegada (AT): ", procesos[i].pid);
        scanf("%d", &procesos[i].llegada);

        printf("Proceso P%d - ráfaga de CPU (BT): ", procesos[i].pid);
        scanf("%d", &procesos[i].rafaga);

        procesos[i].inicio = 0;
        procesos[i].fin = 0;
        procesos[i].espera = 0;
        procesos[i].retorno = 0;
        procesos[i].respuesta = 0;
    }

    return n;
}

static int comparar_por_llegada(const void *a, const void *b) {
    const Proceso *pa = a;
    const Proceso *pb = b;

    if (pa->llegada != pb->llegada) {
        return pa->llegada - pb->llegada;
    }
    return pa->pid - pb->pid;
}

void ordenar_por_llegada(Proceso procesos[], int n) {
    qsort(procesos, (size_t) n, sizeof(Proceso), comparar_por_llegada);
}

void calcular_metricas(Proceso *p) {
    p->retorno = p->fin - p->llegada;
    p->espera = p->retorno - p->rafaga;
    p->respuesta = p->inicio - p->llegada;
}

void imprimir_tabla(const Proceso procesos[], int n) {
    double suma_espera = 0.0;
    double suma_retorno = 0.0;

    printf("%3s %3s %3s %6s %3s %6s %7s\n",
           "PID", "AT", "BT", "Inicio", "Fin", "Espera", "Retorno");

    for (int i = 0; i < n; i++) {
        const Proceso *p = &procesos[i];

        printf("%3d %3d %3d %6d %3d %6d %7d\n",
               p->pid, p->llegada, p->rafaga, p->inicio, p->fin,
               p->espera, p->retorno);

        suma_espera += p->espera;
        suma_retorno += p->retorno;
    }

    if (n > 0) {
        printf("Promedio %-7s: %.2f\n", "espera", suma_espera / n);
        printf("Promedio %-7s: %.2f\n", "retorno", suma_retorno / n);
    }
}
