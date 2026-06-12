/*
 * planificador.c
 * Implementación de las simulaciones de FCFS, SJF, Round Robin y
 * Prioridades sobre ProcesoRR (ver planificador.h).
 */

#include <limits.h>
#include <stdlib.h>

#include "gantt.h"
#include "planificador.h"
#include "proceso.h"

static int comparar_por_llegada_rr(const void *a, const void *b) {
    const ProcesoRR *pa = a;
    const ProcesoRR *pb = b;

    if (pa->llegada != pb->llegada) {
        return pa->llegada - pb->llegada;
    }
    return pa->pid - pb->pid;
}

void simular_fcfs_rr(ProcesoRR procesos[], int n, Gantt *gantt) {
    qsort(procesos, (size_t) n, sizeof(ProcesoRR), comparar_por_llegada_rr);

    gantt_iniciar(gantt);

    int t = 0;
    for (int i = 0; i < n; i++) {
        ProcesoRR *p = &procesos[i];

        if (t < p->llegada) {
            gantt_agregar(gantt, 0, t, p->llegada); /* CPU inactiva */
            t = p->llegada;
        }

        p->inicio = t;
        p->fin = t + p->rafaga;
        calcular_metricas_rr(p);

        gantt_agregar(gantt, p->pid, p->inicio, p->fin);

        t = p->fin;
    }
}

void simular_sjf_rr(ProcesoRR procesos[], int n, Gantt *gantt) {
    int hecho[MAX_PROCESOS] = {0};

    gantt_iniciar(gantt);

    int t = 0;
    int completados = 0;

    while (completados < n) {
        int sel = -1;

        for (int i = 0; i < n; i++) {
            if (hecho[i] || procesos[i].llegada > t) {
                continue;
            }
            if (sel == -1
                || procesos[i].rafaga < procesos[sel].rafaga
                || (procesos[i].rafaga == procesos[sel].rafaga
                    && procesos[i].llegada < procesos[sel].llegada)
                || (procesos[i].rafaga == procesos[sel].rafaga
                    && procesos[i].llegada == procesos[sel].llegada
                    && procesos[i].pid < procesos[sel].pid)) {
                sel = i;
            }
        }

        if (sel == -1) {
            int proxima_llegada = INT_MAX;
            for (int i = 0; i < n; i++) {
                if (!hecho[i] && procesos[i].llegada < proxima_llegada) {
                    proxima_llegada = procesos[i].llegada;
                }
            }
            gantt_agregar(gantt, 0, t, proxima_llegada);
            t = proxima_llegada;
            continue;
        }

        ProcesoRR *p = &procesos[sel];

        p->inicio = t;
        p->fin = t + p->rafaga;
        calcular_metricas_rr(p);

        gantt_agregar(gantt, p->pid, p->inicio, p->fin);

        t = p->fin;
        hecho[sel] = 1;
        completados++;
    }
}

/* Encola en `cola` (cola circular de tamaño MAX_PROCESOS) todos los
 * procesos que aún no han entrado nunca a la cola y cuya llegada es
 * <= t, en orden de PID (1..n). */
static void encolar_llegadas(ProcesoRR procesos[], int n, int t,
                              int cola[], int *fondo, int *tam,
                              int llego[]) {
    for (int i = 0; i < n; i++) {
        if (!llego[i] && procesos[i].llegada <= t) {
            cola[*fondo] = i;
            *fondo = (*fondo + 1) % MAX_PROCESOS;
            (*tam)++;
            llego[i] = 1;
        }
    }
}

void simular_rr(ProcesoRR procesos[], int n, int quantum, Gantt *gantt,
                 int *cambios_contexto) {
    int cola[MAX_PROCESOS];
    int llego[MAX_PROCESOS] = {0}; /* ya entró a la cola (al menos una vez) */
    int frente = 0, fondo = 0, tam = 0;

    gantt_iniciar(gantt);
    *cambios_contexto = 0;

    int t = 0;
    int ultimo_pid = 0; /* 0 = aún no se ha despachado ningún proceso */
    int completados = 0;

    encolar_llegadas(procesos, n, t, cola, &fondo, &tam, llego);

    while (completados < n) {
        if (tam == 0) {
            int proxima_llegada = INT_MAX;
            for (int i = 0; i < n; i++) {
                if (!llego[i] && procesos[i].llegada < proxima_llegada) {
                    proxima_llegada = procesos[i].llegada;
                }
            }
            gantt_agregar(gantt, 0, t, proxima_llegada); /* CPU inactiva */
            t = proxima_llegada;
            encolar_llegadas(procesos, n, t, cola, &fondo, &tam, llego);
            continue;
        }

        int idx = cola[frente];
        frente = (frente + 1) % MAX_PROCESOS;
        tam--;

        ProcesoRR *p = &procesos[idx];

        if (p->inicio == -1) {
            p->inicio = t;
        }

        int ejecutar = (quantum < p->restante) ? quantum : p->restante;

        if (ultimo_pid != 0 && ultimo_pid != p->pid) {
            (*cambios_contexto)++;
        }
        ultimo_pid = p->pid;

        gantt_agregar(gantt, p->pid, t, t + ejecutar);
        t += ejecutar;
        p->restante -= ejecutar;

        /* Encolar primero a los recién llegados en este instante,
         * antes de reinsertar al proceso desalojado. */
        encolar_llegadas(procesos, n, t, cola, &fondo, &tam, llego);

        if (p->restante > 0) {
            cola[fondo] = idx;
            fondo = (fondo + 1) % MAX_PROCESOS;
            tam++;
        } else {
            p->fin = t;
            calcular_metricas_rr(p);
            completados++;
        }
    }
}

void simular_prioridades(ProcesoRR procesos[], int n, Gantt *gantt) {
    int hecho[MAX_PROCESOS] = {0};

    gantt_iniciar(gantt);

    int t = 0;
    int completados = 0;

    while (completados < n) {
        int sel = -1;

        for (int i = 0; i < n; i++) {
            if (hecho[i] || procesos[i].llegada > t) {
                continue;
            }
            if (sel == -1
                || procesos[i].prioridad < procesos[sel].prioridad
                || (procesos[i].prioridad == procesos[sel].prioridad
                    && procesos[i].llegada < procesos[sel].llegada)
                || (procesos[i].prioridad == procesos[sel].prioridad
                    && procesos[i].llegada == procesos[sel].llegada
                    && procesos[i].pid < procesos[sel].pid)) {
                sel = i;
            }
        }

        if (sel == -1) {
            int proxima_llegada = INT_MAX;
            for (int i = 0; i < n; i++) {
                if (!hecho[i] && procesos[i].llegada < proxima_llegada) {
                    proxima_llegada = procesos[i].llegada;
                }
            }
            gantt_agregar(gantt, 0, t, proxima_llegada);
            t = proxima_llegada;
            continue;
        }

        ProcesoRR *p = &procesos[sel];

        p->inicio = t;
        p->fin = t + p->rafaga;
        calcular_metricas_rr(p);

        gantt_agregar(gantt, p->pid, p->inicio, p->fin);

        t = p->fin;
        hecho[sel] = 1;
        completados++;
    }
}
