/*
 * s2_63_quantums.c
 * Ejercicio 6.3 — Round Robin con q=1, q=3, q=100 -> q=100 ≈ FCFS.
 *
 * Corre Round Robin con tres quantums distintos sobre el mismo dataset
 * y, además, FCFS puro. Imprime cada resultado (tabla + Gantt + cambios
 * de contexto) y al final una tabla comparativa de promedios. Con un
 * quantum mayor o igual que la ráfaga más larga (q=100), ningún proceso
 * agota su quantum, así que no hay desalojos por RR: el orden de
 * ejecución y los tiempos de inicio/fin/espera/retorno de cada proceso
 * coinciden exactamente con FCFS (los cambios de contexto que quedan son
 * solo los pases naturales entre procesos al completarse).
 *
 * Compilar: make ejercicios
 *   (equivalente a: gcc -Wall -Wextra -std=c11 -Icommon
 *    ejercicios/s2_63_quantums.c common/proceso.c common/gantt.c
 *    common/planificador.c -o bin/s2_63_quantums)
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

static void mostrar(const char *titulo, const ProcesoRR procesos[], int n, const Gantt *g) {
    printf("\n=== %s ===\n\n", titulo);
    imprimir_tabla_rr(procesos, n, 0);
    printf("\nDiagrama de Gantt:\n");
    gantt_imprimir(g);
}

int main(void) {
    /*           PID  AT  BT  restante prioridad inicio fin espera retorno */
    ProcesoRR base[N_PROCESOS] = {
        {1, 0, 5, 5, 0, -1, 0, 0, 0},
        {2, 1, 3, 3, 0, -1, 0, 0, 0},
        {3, 2, 1, 1, 0, -1, 0, 0, 0},
        {4, 3, 2, 2, 0, -1, 0, 0, 0},
        {5, 4, 4, 4, 0, -1, 0, 0, 0},
    };

    printf("=== Dataset ===\n\n");
    printf("%3s %3s %3s\n", "PID", "AT", "BT");
    for (int i = 0; i < N_PROCESOS; i++) {
        printf("%3d %3d %3d\n", base[i].pid, base[i].llegada, base[i].rafaga);
    }

    ProcesoRR rr1[N_PROCESOS], rr3[N_PROCESOS], rr100[N_PROCESOS], fcfs[N_PROCESOS];
    Gantt g1, g3, g100, gf;
    int cc1, cc3, cc100;

    preparar_copia(base, rr1, N_PROCESOS);
    simular_rr(rr1, N_PROCESOS, 1, &g1, &cc1);
    mostrar("Round Robin (q=1)", rr1, N_PROCESOS, &g1);
    printf("\nCambios de contexto: %d\n", cc1);

    preparar_copia(base, rr3, N_PROCESOS);
    simular_rr(rr3, N_PROCESOS, 3, &g3, &cc3);
    mostrar("Round Robin (q=3)", rr3, N_PROCESOS, &g3);
    printf("\nCambios de contexto: %d\n", cc3);

    preparar_copia(base, rr100, N_PROCESOS);
    simular_rr(rr100, N_PROCESOS, 100, &g100, &cc100);
    mostrar("Round Robin (q=100)", rr100, N_PROCESOS, &g100);
    printf("\nCambios de contexto: %d\n", cc100);

    preparar_copia(base, fcfs, N_PROCESOS);
    simular_fcfs_rr(fcfs, N_PROCESOS, &gf);
    mostrar("FCFS", fcfs, N_PROCESOS, &gf);

    double e1, r1, e3, r3, e100, r100, ef, rf;
    promedios(rr1, N_PROCESOS, &e1, &r1);
    promedios(rr3, N_PROCESOS, &e3, &r3);
    promedios(rr100, N_PROCESOS, &e100, &r100);
    promedios(fcfs, N_PROCESOS, &ef, &rf);

    printf("\n=== Tabla comparativa ===\n\n");
    printf("%-12s %12s %14s %18s\n", "Algoritmo", "Espera prom.", "Retorno prom.", "Cambios contexto");
    printf("%-12s %12.2f %14.2f %18d\n", "RR (q=1)", e1, r1, cc1);
    printf("%-12s %12.2f %14.2f %18d\n", "RR (q=3)", e3, r3, cc3);
    printf("%-12s %12.2f %14.2f %18d\n", "RR (q=100)", e100, r100, cc100);
    printf("%-12s %12.2f %14.2f %18s\n", "FCFS", ef, rf, "-");

    int iguales = (e100 == ef && r100 == rf);
    for (int i = 0; i < N_PROCESOS && iguales; i++) {
        if (rr100[i].espera != fcfs[i].espera || rr100[i].retorno != fcfs[i].retorno
            || rr100[i].inicio != fcfs[i].inicio || rr100[i].fin != fcfs[i].fin) {
            iguales = 0;
        }
    }

    printf("\nConclusión: con q=100 (mayor que cualquier ráfaga), ningún\n");
    printf("proceso agota su quantum antes de terminar, por lo que no hay\n");
    printf("desalojos por RR. Los %d cambios de contexto restantes son solo\n", cc100);
    printf("los pases naturales de un proceso a otro al completarse (igual\n");
    printf("que ocurriría en FCFS). Inicio/fin/espera/retorno por proceso son\n");
    printf("%s a los de FCFS.\n", iguales ? "IDÉNTICOS" : "DISTINTOS");

    return 0;
}
