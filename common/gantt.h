/*
 * gantt.h
 * Generador de diagramas de Gantt en texto para los algoritmos de
 * planificación. Agrupa automáticamente ráfagas consecutivas del mismo
 * PID en un solo bloque.
 *
 * Salida con el formato:
 *   | P1 | P2 | P3 |
 *   0    5    8    16
 */

#ifndef GANTT_H
#define GANTT_H

#define GANTT_MAX_EVENTOS 1000

typedef struct {
    int pid;     /* PID en ejecución; 0 = CPU inactiva (IDLE) */
    int inicio;  /* instante en que comienza el tramo          */
    int fin;     /* instante en que termina el tramo           */
} EventoGantt;

typedef struct {
    EventoGantt eventos[GANTT_MAX_EVENTOS];
    int n;
} Gantt;

/* Inicializa (vacía) el diagrama de Gantt. */
void gantt_iniciar(Gantt *g);

/* Agrega el tramo [t_inicio, t_fin) en el que ejecuta `pid` (0 = IDLE).
 * Si el tramo es consecutivo y del mismo PID que el último agregado,
 * se fusiona con él (agrupando ráfagas consecutivas). */
void gantt_agregar(Gantt *g, int pid, int t_inicio, int t_fin);

/* Imprime el diagrama: una línea con las barras "| Px |" y otra debajo
 * con la línea de tiempos de inicio de cada tramo y el tiempo final. */
void gantt_imprimir(const Gantt *g);

#endif /* GANTT_H */
