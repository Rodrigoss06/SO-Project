/*
 * planificador.h
 * Simulaciones de planificación sobre ProcesoRR, reutilizadas por los
 * programas de la Sesión 2 (round_robin.c, prioridades.c) y por
 * comparativa.c, que corre los 4 algoritmos sobre el mismo dataset.
 *
 * Cada función deja en `procesos[]` los campos inicio/fin/espera/
 * retorno calculados y llena `gantt` con el diagrama de ejecución.
 */

#ifndef PLANIFICADOR_H
#define PLANIFICADOR_H

#include "gantt.h"
#include "proceso.h"

/* FCFS sobre ProcesoRR: ordena por llegada (desempate por PID) y
 * ejecuta cada proceso de forma no apropiativa en ese orden. */
void simular_fcfs_rr(ProcesoRR procesos[], int n, Gantt *gantt);

/* SJF no apropiativo sobre ProcesoRR: en cada paso elige la menor
 * ráfaga entre los procesos ya llegados (desempate por AT y luego por
 * PID); si no hay disponibles, avanza el tiempo (CPU inactiva). */
void simular_sjf_rr(ProcesoRR procesos[], int n, Gantt *gantt);

/* Round Robin: cola circular FIFO con quantum fijo. En cada despacho
 * ejecuta min(quantum, restante). Si el proceso no termina, primero se
 * encolan los procesos recién llegados en ese instante y luego se
 * reinserta el proceso desalojado al final de la cola. Cuenta en
 * `*cambios_contexto` cada vez que el PID despachado difiere del
 * anterior (sin contar el primer despacho). */
void simular_rr(ProcesoRR procesos[], int n, int quantum, Gantt *gantt,
                 int *cambios_contexto);

/* Prioridades no apropiativo: en cada paso elige la mayor prioridad
 * (menor número) entre los procesos ya llegados; empate -> FCFS (menor
 * AT y, si persiste, menor PID). Si no hay disponibles, avanza el
 * tiempo (CPU inactiva). */
void simular_prioridades(ProcesoRR procesos[], int n, Gantt *gantt);

#endif /* PLANIFICADOR_H */
