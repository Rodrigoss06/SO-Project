/*
 * proceso.h
 * Estructura de datos compartida y utilidades para los algoritmos de
 * planificación (Sesión 1: FCFS y SJF; Sesión 2: Round Robin y
 * Prioridades; y siguientes sesiones).
 *
 * Incluye:
 *   - struct Proceso con los campos AT, BT, CT, TAT, WT y RT.
 *   - struct ProcesoRR (añade `restante` y `prioridad` para RR y
 *     Prioridades).
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

/* Proceso con campos adicionales para Round Robin (restante) y
 * Prioridades (prioridad, 1 = mayor .. 5 = menor). */
typedef struct {
    int pid;
    int llegada;  /* AT                                    */
    int rafaga;   /* BT                                    */
    int restante; /* ráfaga restante (se consume en RR)    */
    int prioridad; /* 1..5, 1 = mayor prioridad            */
    int inicio;   /* primer instante en que usa la CPU; -1 = aún no */
    int fin;      /* CT                                    */
    int espera;   /* WT = TAT - BT                         */
    int retorno;  /* TAT = CT - AT                         */
} ProcesoRR;

/* Pide por teclado la cantidad de procesos (hasta `max`) y, para cada
 * uno, su tiempo de llegada (AT) y ráfaga (BT). Si `con_prioridad` es
 * distinto de 0, también pide una prioridad entre 1 (mayor) y 5
 * (menor); en caso contrario la deja en 0. Inicializa `restante` =
 * `rafaga` e `inicio` = -1. Asigna PID = 1..N según el orden de
 * ingreso. Devuelve la cantidad de procesos leídos. */
int leer_procesos_rr(ProcesoRR procesos[], int max, int con_prioridad);

/* Calcula TAT y WT de un ProcesoRR a partir de sus campos llegada,
 * rafaga y fin (que deben estar ya asignados). */
void calcular_metricas_rr(ProcesoRR *p);

/* Imprime la tabla de resultados para ProcesoRR y los promedios de
 * espera y retorno. Si `mostrar_prioridad` es distinto de 0, incluye
 * la columna de prioridad. */
void imprimir_tabla_rr(const ProcesoRR procesos[], int n, int mostrar_prioridad);

#endif /* PROCESO_H */
