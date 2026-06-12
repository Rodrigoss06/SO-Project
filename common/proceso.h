/*
 * proceso.h
 * Estructura de datos compartida y utilidades para los algoritmos de
 * planificación (Sesión 1: FCFS y SJF, y siguientes sesiones).
 *
 * Incluye:
 *   - struct Proceso con los campos AT, BT, CT, TAT, WT y RT.
 *   - Lectura interactiva de procesos.
 *   - Ordenamiento por tiempo de llegada (desempate por PID).
 *   - Cálculo de métricas (TAT, WT, RT) a partir de inicio/fin.
 *   - Impresión de tabla de resultados con promedios.
 */

#ifndef PROCESO_H
#define PROCESO_H

#define MAX_PROCESOS 100

typedef struct {
    int pid;       /* identificador del proceso            */
    int llegada;   /* AT - tiempo de llegada                */
    int rafaga;    /* BT - ráfaga de CPU                    */
    int inicio;    /* primer instante en que usa la CPU     */
    int fin;       /* CT - instante de finalización         */
    int espera;    /* WT = TAT - BT                         */
    int retorno;   /* TAT = CT - AT                         */
    int respuesta; /* RT = inicio - AT                      */
} Proceso;

/* Pide por teclado la cantidad de procesos (hasta `max`) y, para cada uno,
 * su tiempo de llegada (AT) y ráfaga (BT). Asigna PID = 1..N según el orden
 * de ingreso. Devuelve la cantidad de procesos leídos. */
int leer_procesos(Proceso procesos[], int max);

/* Ordena el arreglo de procesos por tiempo de llegada (AT) ascendente,
 * desempatando por PID ascendente. */
void ordenar_por_llegada(Proceso procesos[], int n);

/* Calcula TAT, WT y RT de un proceso a partir de sus campos
 * llegada, rafaga, inicio y fin (que deben estar ya asignados). */
void calcular_metricas(Proceso *p);

/* Imprime la tabla PID/AT/BT/Inicio/Fin/Espera/Retorno y, al final,
 * los promedios de espera y retorno. */
void imprimir_tabla(const Proceso procesos[], int n);

#endif /* PROCESO_H */
