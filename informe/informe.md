# Informe — Lab N°09: Mini Sistema Operativo (Scheduling + Concurrencia)

## 1. Carátula

- **Curso:** Sistemas Operativos
- **Laboratorio:** N°09 — Mini Sistema Operativo (Planificación de CPU + Concurrencia)
- **Docente:** Ing. Gustavo Reinoso
- **Universidad:** Universidad Católica de Santa María (UCSM)
- **Estudiante:** [Nombre completo del estudiante]
- **Fecha:** 12 de junio de 2026
- **Repositorio:** `mini-so-lab09` (rama `master`)

[CAPTURA: caratula_repo.png]

---

## 2. Objetivos

1. Implementar y comparar los algoritmos de planificación de CPU **FCFS**,
   **SJF**, **Round Robin** y **Prioridades**, calculando las métricas
   estándar (`AT`, `BT`, `CT`, `TAT`, `WT`, `RT`) y representando la
   ejecución mediante diagramas de Gantt en texto.
2. Aplicar **hilos POSIX (pthreads)**, **mutex** y **semáforos** para
   resolver problemas clásicos de concurrencia: condición de carrera y
   productor-consumidor.
3. Integrar planificación y concurrencia en un **simulador** realista
   (escenario Banco/Caja) que modela un recurso limitado con un semáforo
   contador y calcula métricas de espera y uso del recurso.
4. Resolver los **12 ejercicios propuestos** (Sesiones 1-3) para reforzar
   los desempates de los algoritmos, el efecto convoy, RR apropiativo con
   prioridades, el efecto del quantum, productor-consumidor con múltiples
   hilos, medición del costo de la sincronización y paralelismo con hilos.
5. Documentar el código, los resultados (tablas, Gantt, logs) y reflexionar
   sobre la relación entre estos algoritmos y un sistema operativo real.

---

## 3. Recursos y entorno

```text
$ gcc --version
gcc (GCC) 16.1.1 20260430

$ cat /etc/os-release | head -1
NAME="Arch Linux"

$ uname -r
7.0.9-arch1-1
```

- **Compilador:** `gcc`, estándar **C11**.
- **Flags obligatorios:** `-Wall -Wextra -std=c11` (cero warnings); los
  programas de Sesión 3 y 4 además agregan `-pthread`.
- **Build:** `make`, `make s1`, `make s2`, `make s3`, `make s4`,
  `make ejercicios`, `make clean` — todos los binarios se generan en
  `./bin/`.
- **Verificación de concurrencia:** `ps -eLf`, `top -H` y, donde está
  disponible, `valgrind --tool=helgrind`.

[CAPTURA: make_build.png]

### Módulo compartido `common/`

Para no duplicar lógica entre sesiones, toda la representación de
procesos, el cálculo de métricas, la impresión de tablas y el diagrama de
Gantt viven en `common/`:

- **`common/proceso.h` / `proceso.c`** — `struct Proceso` (Sesión 1, campos
  `pid, llegada, rafaga, inicio, fin, espera, retorno, respuesta`) y
  `struct ProcesoRR` (Sesión 2+, agrega `restante` y `prioridad`).
  Funciones: `leer_procesos`, `leer_procesos_rr`, `ordenar_por_llegada`,
  `calcular_metricas`, `calcular_metricas_rr`, `imprimir_tabla`,
  `imprimir_tabla_rr`.
- **`common/gantt.h` / `gantt.c`** — `gantt_iniciar`, `gantt_agregar`
  (fusiona tramos consecutivos del mismo PID) y `gantt_imprimir`, que
  produce el formato:

  ```text
  | P1 | P2 | P3 |
  0    5    8    16
  ```

- **`common/planificador.h` / `planificador.c`** — `simular_fcfs_rr`,
  `simular_sjf_rr`, `simular_rr` (con quantum y conteo de cambios de
  contexto) y `simular_prioridades`, todas operando sobre `ProcesoRR[]`.
  Reutilizadas por Sesión 2 y por varios de los ejercicios.

**Métricas estándar usadas en todo el informe:**

| Sigla | Nombre | Fórmula |
|-------|--------|---------|
| AT | Tiempo de llegada (Arrival Time) | dato de entrada |
| BT | Ráfaga de CPU (Burst Time) | dato de entrada |
| CT | Tiempo de finalización (Completion Time) | — |
| TAT | Tiempo de retorno (Turnaround Time) | `CT - AT` |
| WT | Tiempo de espera (Waiting Time) | `TAT - BT` |
| RT | Tiempo de respuesta (Response Time) | `inicio - AT` |

---

## 4. Sesión 1 — FCFS y SJF

### 4.1 Dataset usado

Para poder comparar FCFS y SJF sobre los mismos datos (y reutilizarlo en la
Sesión 2), se usa el siguiente conjunto de 5 procesos:

| PID | AT | BT |
|-----|----|----|
| P1  | 0  | 5  |
| P2  | 1  | 3  |
| P3  | 2  | 1  |
| P4  | 3  | 2  |
| P5  | 4  | 4  |

### 4.2 Actividad A1 — FCFS (`sesion1-scheduling/fcfs.c`)

```c
/*
 * fcfs.c
 * Sesión 1 — Actividad A1: planificación FCFS (First Come First Served).
 *
 * Pide N procesos (AT, BT), los ordena por tiempo de llegada (desempate
 * por PID) y los ejecuta en ese orden sin apropiación. Calcula las
 * métricas (TAT, WT, RT), imprime la tabla con promedios y el diagrama
 * de Gantt.
 *
 * Compilar: make s1
 *   (equivalente a: gcc -Wall -Wextra -std=c11 -Icommon
 *    sesion1-scheduling/fcfs.c common/proceso.c common/gantt.c -o bin/fcfs)
 */

#include <stdio.h>

#include "gantt.h"
#include "proceso.h"

int main(void) {
    Proceso procesos[MAX_PROCESOS];
    int n = leer_procesos(procesos, MAX_PROCESOS);

    ordenar_por_llegada(procesos, n);

    Gantt gantt;
    gantt_iniciar(&gantt);

    int t = 0;
    for (int i = 0; i < n; i++) {
        Proceso *p = &procesos[i];

        if (t < p->llegada) {
            gantt_agregar(&gantt, 0, t, p->llegada); /* CPU inactiva */
            t = p->llegada;
        }

        p->inicio = t;
        p->fin = t + p->rafaga;
        calcular_metricas(p);

        gantt_agregar(&gantt, p->pid, p->inicio, p->fin);

        t = p->fin;
    }

    printf("\n=== FCFS (First Come First Served) ===\n\n");
    imprimir_tabla(procesos, n);

    printf("\nDiagrama de Gantt:\n");
    gantt_imprimir(&gantt);

    return 0;
}
```

**Ejecución** (`./bin/fcfs`, dataset de la sección 4.1):

```text
=== FCFS (First Come First Served) ===

PID  AT  BT Inicio Fin Espera Retorno
  1   0   5      0   5      0       5
  2   1   3      5   8      4       7
  3   2   1      8   9      6       7
  4   3   2      9  11      6       8
  5   4   4     11  15      7      11
Promedio espera : 4.60
Promedio retorno: 7.60

Diagrama de Gantt:
| P1 | P2 | P3 | P4 | P5 |
0    5    8    9    11   15
```

[CAPTURA: s1_fcfs_exec.png]

### 4.3 Actividad A2 — SJF no apropiativo (`sesion1-scheduling/sjf.c`)

```c
/*
 * sjf.c
 * Sesión 1 — Actividad A2: planificación SJF no apropiativo
 * (Shortest Job First).
 *
 * Pide N procesos (AT, BT). En cada paso elige, entre los procesos ya
 * llegados (AT <= t) y no ejecutados, el de menor ráfaga (desempate por
 * AT y luego por PID). Si no hay procesos disponibles, avanza `t` hasta
 * la próxima llegada (CPU inactiva). Calcula métricas (TAT, WT, RT),
 * imprime la tabla con promedios y el diagrama de Gantt.
 *
 * Compilar: make s1
 *   (equivalente a: gcc -Wall -Wextra -std=c11 -Icommon
 *    sesion1-scheduling/sjf.c common/proceso.c common/gantt.c -o bin/sjf)
 */

#include <limits.h>
#include <stdio.h>

#include "gantt.h"
#include "proceso.h"

int main(void) {
    Proceso procesos[MAX_PROCESOS];
    int n = leer_procesos(procesos, MAX_PROCESOS);

    int hecho[MAX_PROCESOS] = {0};

    Gantt gantt;
    gantt_iniciar(&gantt);

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
            /* No hay procesos disponibles: CPU inactiva hasta la
             * próxima llegada. */
            int proxima_llegada = INT_MAX;
            for (int i = 0; i < n; i++) {
                if (!hecho[i] && procesos[i].llegada < proxima_llegada) {
                    proxima_llegada = procesos[i].llegada;
                }
            }
            gantt_agregar(&gantt, 0, t, proxima_llegada);
            t = proxima_llegada;
            continue;
        }

        Proceso *p = &procesos[sel];

        p->inicio = t;
        p->fin = t + p->rafaga;
        calcular_metricas(p);

        gantt_agregar(&gantt, p->pid, p->inicio, p->fin);

        t = p->fin;
        hecho[sel] = 1;
        completados++;
    }

    printf("\n=== SJF (Shortest Job First, no apropiativo) ===\n\n");
    imprimir_tabla(procesos, n);

    printf("\nDiagrama de Gantt:\n");
    gantt_imprimir(&gantt);

    return 0;
}
```

**Ejecución** (`./bin/sjf`, mismo dataset):

```text
=== SJF (Shortest Job First, no apropiativo) ===

PID  AT  BT Inicio Fin Espera Retorno
  1   0   5      0   5      0       5
  2   1   3      8  11      7      10
  3   2   1      5   6      3       4
  4   3   2      6   8      3       5
  5   4   4     11  15      7      11
Promedio espera : 4.00
Promedio retorno: 7.00

Diagrama de Gantt:
| P1 | P3 | P4 | P2 | P5 |
0    5    6    8    11   15
```

[CAPTURA: s1_sjf_exec.png]

### 4.4 Comparación FCFS vs. SJF

| Algoritmo | Espera prom. | Retorno prom. |
|-----------|--------------|----------------|
| FCFS      | 4.60         | 7.60           |
| SJF       | 4.00         | 7.00           |

SJF reduce tanto la espera como el retorno promedio frente a FCFS sobre el
mismo dataset, porque P1 (BT=5, el más largo) bloquea a los procesos más
cortos que llegan justo después en FCFS; SJF intercala P3 (BT=1) entre P1 y
el resto, adelantando trabajo corto. Esta misma idea se profundiza en el
ejercicio 5.3 (efecto convoy).

### 4.5 Ejercicios propuestos — Sesión 1 (5.1 a 5.4)

#### 5.1 — `ejercicios/s1_51_aleatorio.c`: FCFS vs SJF con 6 procesos aleatorios

Genera 6 procesos con `AT` (0..9) y `BT` (1..8) aleatorios (`srand`, admite
`--seed N`) y corre FCFS y SJF sobre el mismo dataset, mostrando ambas
tablas, sus Gantt y una tabla resumen de promedios.

**Ejecución** (`./bin/s1_51_aleatorio --seed 42`):

```text
=== Procesos generados (semilla 42) ===

PID  AT  BT
  1   6   5
  2   1   2
  3   2   7
  4   1   5
  5   5   8
  6   4   8

=== FCFS ===

PID  AT  BT Inicio Fin Espera Retorno
  2   1   2      1   3      0       2
  4   1   5      3   8      2       7
  3   2   7      8  15      6      13
  6   4   8     15  23     11      19
  5   5   8     23  31     18      26
  1   6   5     31  36     25      30
Promedio espera : 10.33
Promedio retorno: 16.17

Diagrama de Gantt:
| IDLE | P2 | P4 | P3 | P6 | P5 | P1 |
0      1    3    8    15   23   31   36

=== SJF (no apropiativo) ===

PID  AT  BT Inicio Fin Espera Retorno
  1   6   5      8  13      2       7
  2   1   2      1   3      0       2
  3   2   7     13  20     11      18
  4   1   5      3   8      2       7
  5   5   8     28  36     23      31
  6   4   8     20  28     16      24
Promedio espera : 9.00
Promedio retorno: 14.83

Diagrama de Gantt:
| IDLE | P2 | P4 | P1 | P3 | P6 | P5 |
0      1    3    8    13   20   28   36

=== Tabla resumen ===

Algoritmo    Espera prom.  Retorno prom.
FCFS                10.33          16.17
SJF                  9.00          14.83
```

[CAPTURA: ej_5_1_exec.png]

Código completo: `ejercicios/s1_51_aleatorio.c` (reutiliza `preparar_copia`
y `promedios`, igual que `comparativa.c`, sobre `simular_fcfs_rr` y
`simular_sjf_rr` de `common/planificador.c`).

#### 5.2 — `ejercicios/s1_52_desempate.c`: SJF con desempate (ráfaga → AT → PID)

Dataset fijo diseñado para forzar los dos primeros empates posibles de
`simular_sjf_rr`: empate de ráfaga con igual `AT` (se resuelve por PID) y
empate de ráfaga con distinto `AT` (se resuelve por menor `AT`).

| PID | AT | BT |
|-----|----|----|
| P1 | 0 | 6 |
| P2 | 0 | 2 |
| P3 | 0 | 2 |
| P4 | 1 | 2 |
| P5 | 3 | 1 |
| P6 | 1 | 4 |

**Ejecución** (`./bin/s1_52_desempate`):

```text
=== SJF (no apropiativo) ===

PID  AT  BT Inicio Fin Espera Retorno
  1   0   6     11  17     11      17
  2   0   2      0   2      0       2
  3   0   2      2   4      2       4
  4   1   2      5   7      4       6
  5   3   1      4   5      1       2
  6   1   4      7  11      6      10
Promedio espera : 4.00
Promedio retorno: 6.83

Diagrama de Gantt:
| P2 | P3 | P5 | P4 | P6 | P1 |
0    2    4    5    7    11   17

=== Traza de desempates ===

t=0 : disponibles P1(BT6,AT0), P2(BT2,AT0), P3(BT2,AT0).
      Empate de menor BT entre P2 y P3 (ambos BT=2, AT=0)
      -> se desempata por PID -> elige P2.

t=2 : disponibles P1(BT6,AT0), P3(BT2,AT0), P4(BT2,AT1), P6(BT4,AT1).
      Empate de menor BT entre P3 y P4 (ambos BT=2, AT distinto)
      -> se desempata por AT (menor AT) -> elige P3 (AT=0).

t=4 : disponibles P1(BT6), P4(BT2), P6(BT4), P5(BT1).
      Menor BT sin empate -> elige P5 (BT=1).

t=5 : disponibles P1(BT6), P4(BT2), P6(BT4).
      Menor BT sin empate -> elige P4 (BT=2).

t=7 : disponibles P1(BT6), P6(BT4).
      Menor BT sin empate -> elige P6 (BT=4).

t=11: solo queda P1 -> elige P1 (BT=6).
```

[CAPTURA: ej_5_2_exec.png]

#### 5.3 — `ejercicios/s1_53_convoy.c`: Efecto convoy en FCFS

4 procesos llegan en `t=0`: P1 (BT=10, largo) y P2/P3/P4 (BT=1, cortos). En
FCFS, P1 entra primero por orden de PID (empate de AT=0) y los 3 procesos
cortos quedan en "convoy" detrás de él; SJF evita el efecto ejecutando
primero los cortos.

**Ejecución** (`./bin/s1_53_convoy`):

```text
=== Dataset (los 4 procesos llegan en t=0) ===

PID  AT  BT
  1   0  10
  2   0   1
  3   0   1
  4   0   1

=== FCFS ===

PID  AT  BT Inicio Fin Espera Retorno
  1   0  10      0  10      0      10
  2   0   1     10  11     10      11
  3   0   1     11  12     11      12
  4   0   1     12  13     12      13
Promedio espera : 8.25
Promedio retorno: 11.50

Diagrama de Gantt:
| P1 | P2 | P3 | P4 |
0    10   11   12   13

=== SJF (no apropiativo) ===

PID  AT  BT Inicio Fin Espera Retorno
  1   0  10      3  13      3      13
  2   0   1      0   1      0       1
  3   0   1      1   2      1       2
  4   0   1      2   3      2       3
Promedio espera : 1.50
Promedio retorno: 4.75

Diagrama de Gantt:
| P2 | P3 | P4 | P1 |
0    1    2    3    13

=== Efecto convoy ===

FCFS: P1 (BT=10) entra primero por orden de PID (empate de AT=0)
y P2, P3, P4 (BT=1) quedan en "convoy" detrás de él.
Espera promedio FCFS = 8.25

SJF: los procesos cortos se ejecutan primero; P1 pasa al final.
Espera promedio SJF  = 1.50

Conclusión: bajo FCFS, procesos cortos pueden esperar mucho tiempo
detrás de uno largo (convoy). SJF reduce drásticamente la espera
promedio al priorizar las ráfagas cortas.
```

[CAPTURA: ej_5_3_exec.png]

#### 5.4 — `ejercicios/s1_54_respuesta.c`: Tiempo de respuesta (RT) para FCFS y SJF

`common/proceso.h` no tiene un campo `RT` separado porque, en algoritmos no
apropiativos, cada proceso usa la CPU en un único tramo
`[inicio, fin)`, por lo que `RT = inicio - AT = (CT - BT) - AT
= (CT - AT) - BT = TAT - BT = WT`. Es decir, **RT y WT son siempre iguales
en FCFS y SJF**. Este programa agrega una columna RT calculada como
`inicio - AT` y verifica la igualdad para cada proceso.

**Ejecución** (`./bin/s1_54_respuesta`):

```text
=== Dataset ===

PID  AT  BT
  1   0   4
  2   1   3
  3   2   1
  4   3   2
  5   4   5

=== FCFS (con columna RT) ===

PID  AT  BT Inicio Fin Espera Retorno  RT
  1   0   4      0   4      0       4   0
  2   1   3      4   7      3       6   3
  3   2   1      7   8      5       6   5
  4   3   2      8  10      5       7   5
  5   4   5     10  15      6      11   6

Diagrama de Gantt:
| P1 | P2 | P3 | P4 | P5 |
0    4    7    8    10   15

=== SJF no apropiativo (con columna RT) ===

PID  AT  BT Inicio Fin Espera Retorno  RT
  1   0   4      0   4      0       4   0
  2   1   3      7  10      6       9   6
  3   2   1      4   5      2       3   2
  4   3   2      5   7      2       4   2
  5   4   5     10  15      6      11   6

Diagrama de Gantt:
| P1 | P3 | P4 | P2 | P5 |
0    4    5    7    10   15

Conclusión: en ambos algoritmos RT == WT para todos los
procesos, porque al ser no apropiativos cada proceso usa la
CPU en un único tramo continuo (inicio..fin).
```

[CAPTURA: ej_5_4_exec.png]

**Resumen Sesión 1 — ejercicios:** 4/4 resueltos (5.1, 5.2, 5.3, 5.4).

---

## 5. Sesión 2 — Round Robin y Prioridades

### 5.1 Dataset usado

Mismo dataset de AT/BT que la Sesión 1, agregando una prioridad
(1 = mayor, 5 = menor) para las actividades que la requieren:

| PID | AT | BT | Prioridad |
|-----|----|----|-----------|
| P1  | 0  | 5  | 2 |
| P2  | 1  | 3  | 1 |
| P3  | 2  | 1  | 3 |
| P4  | 3  | 2  | 1 |
| P5  | 4  | 4  | 2 |

### 5.2 Actividad A1 — Round Robin (`sesion2-rr-prioridades/round_robin.c`)

```c
/*
 * round_robin.c
 * Sesión 2 — Actividad A1: planificación Round Robin (RR).
 *
 * Pide N procesos (AT, BT) y un quantum. Usa una cola circular FIFO:
 * en cada despacho ejecuta min(quantum, restante); si el proceso no
 * termina, primero se encolan los procesos recién llegados en ese
 * instante y luego se reinserta el proceso desalojado al final de la
 * cola (convención de reinserción). Calcula CT/WT/TAT, imprime la
 * tabla con promedios, el diagrama de Gantt y el número de cambios de
 * contexto.
 *
 * Compilar: make s2
 *   (equivalente a: gcc -Wall -Wextra -std=c11 -Icommon
 *    sesion2-rr-prioridades/round_robin.c common/proceso.c
 *    common/gantt.c common/planificador.c -o bin/round_robin)
 */

#include <stdio.h>

#include "gantt.h"
#include "planificador.h"
#include "proceso.h"

int main(void) {
    ProcesoRR procesos[MAX_PROCESOS];
    int n = leer_procesos_rr(procesos, MAX_PROCESOS, 0);

    int quantum;
    printf("Quantum: ");
    while (scanf("%d", &quantum) != 1 || quantum <= 0) {
        printf("Ingrese un quantum mayor que 0: ");
        while (getchar() != '\n') {
            /* descarta el resto de la línea inválida */
        }
    }

    Gantt gantt;
    int cambios_contexto;
    simular_rr(procesos, n, quantum, &gantt, &cambios_contexto);

    printf("\n=== Round Robin (quantum = %d) ===\n\n", quantum);
    imprimir_tabla_rr(procesos, n, 0);

    printf("\nDiagrama de Gantt:\n");
    gantt_imprimir(&gantt);

    printf("\nCambios de contexto: %d\n", cambios_contexto);

    return 0;
}
```

**Ejecución** (`./bin/round_robin`, dataset 5.1, quantum = 2):

```text
=== Round Robin (quantum = 2) ===

PID  AT  BT Inicio Fin Espera Retorno
  1   0   5      0  13      8      13
  2   1   3      2  12      8      11
  3   2   1      4   5      2       3
  4   3   2      7   9      4       6
  5   4   4      9  15      7      11
Promedio espera : 5.80
Promedio retorno: 8.80

Diagrama de Gantt:
| P1 | P2 | P3 | P1 | P4 | P5 | P2 | P1 | P5 |
0    2    4    5    7    9    11   12   13   15

Cambios de contexto: 8
```

[CAPTURA: s2_round_robin_exec.png]

**Convención de reinserción (gotcha documentado en `CLAUDE.md`):** cuando un
proceso agota su quantum en el mismo instante `t` en que llega uno nuevo,
primero se encola el recién llegado y luego el desalojado, para no
favorecer artificialmente a quien ya tuvo su turno.

### 5.3 Actividad A2 — Prioridades no apropiativo (`sesion2-rr-prioridades/prioridades.c`)

```c
/*
 * prioridades.c
 * Sesión 2 — Actividad A2: planificación por Prioridades, no
 * apropiativo.
 *
 * Pide N procesos (AT, BT, prioridad 1..5; 1 = mayor). En cada paso
 * elige, entre los procesos ya llegados (AT <= t) y no ejecutados, el
 * de mayor prioridad (menor número); empate -> FCFS (menor AT y, si
 * persiste, menor PID). Si no hay disponibles, avanza `t` hasta la
 * próxima llegada (CPU inactiva). Calcula CT/WT/TAT, imprime la tabla
 * con promedios y el diagrama de Gantt.
 *
 * Compilar: make s2
 *   (equivalente a: gcc -Wall -Wextra -std=c11 -Icommon
 *    sesion2-rr-prioridades/prioridades.c common/proceso.c
 *    common/gantt.c common/planificador.c -o bin/prioridades)
 */

#include <stdio.h>

#include "gantt.h"
#include "planificador.h"
#include "proceso.h"

int main(void) {
    ProcesoRR procesos[MAX_PROCESOS];
    int n = leer_procesos_rr(procesos, MAX_PROCESOS, 1);

    Gantt gantt;
    simular_prioridades(procesos, n, &gantt);

    printf("\n=== Prioridades (no apropiativo) ===\n\n");
    imprimir_tabla_rr(procesos, n, 1);

    printf("\nDiagrama de Gantt:\n");
    gantt_imprimir(&gantt);

    return 0;
}
```

**Ejecución** (`./bin/prioridades`, dataset 5.1 con prioridades):

```text
=== Prioridades (no apropiativo) ===

PID  AT  BT Prio Inicio Fin Espera Retorno
  1   0   5    2      0   5      0       5
  2   1   3    1      5   8      4       7
  3   2   1    3     14  15     12      13
  4   3   2    1      8  10      5       7
  5   4   4    2     10  14      6      10
Promedio espera : 5.40
Promedio retorno: 8.40

Diagrama de Gantt:
| P1 | P2 | P4 | P5 | P3 |
0    5    8    10   14   15
```

[CAPTURA: s2_prioridades_exec.png]

P3 (prioridad 3, la menor) queda relegada al final pese a llegar en `t=2`
con una ráfaga muy corta (BT=1): es el caso opuesto a SJF, donde P3 habría
entrado mucho antes. Esto motiva el ejercicio 6.1 (aging) para evitar la
inanición de procesos de baja prioridad.

### 5.4 Actividad A3 — Comparativa (`sesion2-rr-prioridades/comparativa.c`)

```c
/*
 * comparativa.c
 * Sesión 2 — Actividad A3: comparación de FCFS, SJF, Round Robin
 * (q=2 y q=4) y Prioridades sobre el mismo dataset (AT, BT,
 * prioridad).
 *
 * Para cada algoritmo imprime la tabla de resultados y el diagrama de
 * Gantt, y al final arma la tabla comparativa de espera promedio,
 * retorno promedio y número de cambios de contexto (solo aplica a
 * Round Robin; en el resto se muestra "-").
 *
 * Compilar: make s2
 *   (equivalente a: gcc -Wall -Wextra -std=c11 -Icommon
 *    sesion2-rr-prioridades/comparativa.c common/proceso.c
 *    common/gantt.c common/planificador.c -o bin/comparativa)
 */

#include <stdio.h>

#include "gantt.h"
#include "planificador.h"
#include "proceso.h"

/* Copia el dataset base a una copia de trabajo, reiniciando los
 * campos que cada simulación va a recalcular. */
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

static void mostrar_resultado(const char *titulo, const ProcesoRR procesos[], int n,
                               const Gantt *gantt) {
    printf("\n=== %s ===\n\n", titulo);
    imprimir_tabla_rr(procesos, n, 1);
    printf("\nDiagrama de Gantt:\n");
    gantt_imprimir(gantt);
}

int main(void) {
    ProcesoRR base[MAX_PROCESOS];
    int n = leer_procesos_rr(base, MAX_PROCESOS, 1);

    ProcesoRR fcfs[MAX_PROCESOS];
    ProcesoRR sjf[MAX_PROCESOS];
    ProcesoRR rr2[MAX_PROCESOS];
    ProcesoRR rr4[MAX_PROCESOS];
    ProcesoRR prio[MAX_PROCESOS];

    Gantt g_fcfs, g_sjf, g_rr2, g_rr4, g_prio;
    int cc_rr2, cc_rr4;

    preparar_copia(base, fcfs, n);
    simular_fcfs_rr(fcfs, n, &g_fcfs);
    mostrar_resultado("FCFS", fcfs, n, &g_fcfs);

    preparar_copia(base, sjf, n);
    simular_sjf_rr(sjf, n, &g_sjf);
    mostrar_resultado("SJF (no apropiativo)", sjf, n, &g_sjf);

    preparar_copia(base, rr2, n);
    simular_rr(rr2, n, 2, &g_rr2, &cc_rr2);
    mostrar_resultado("Round Robin (quantum = 2)", rr2, n, &g_rr2);
    printf("\nCambios de contexto: %d\n", cc_rr2);

    preparar_copia(base, rr4, n);
    simular_rr(rr4, n, 4, &g_rr4, &cc_rr4);
    mostrar_resultado("Round Robin (quantum = 4)", rr4, n, &g_rr4);
    printf("\nCambios de contexto: %d\n", cc_rr4);

    preparar_copia(base, prio, n);
    simular_prioridades(prio, n, &g_prio);
    mostrar_resultado("Prioridades (no apropiativo)", prio, n, &g_prio);

    double esp_fcfs, ret_fcfs, esp_sjf, ret_sjf;
    double esp_rr2, ret_rr2, esp_rr4, ret_rr4, esp_prio, ret_prio;

    promedios(fcfs, n, &esp_fcfs, &ret_fcfs);
    promedios(sjf, n, &esp_sjf, &ret_sjf);
    promedios(rr2, n, &esp_rr2, &ret_rr2);
    promedios(rr4, n, &esp_rr4, &ret_rr4);
    promedios(prio, n, &esp_prio, &ret_prio);

    printf("\n=== Tabla comparativa ===\n\n");
    printf("%-12s %12s %14s %20s\n",
           "Algoritmo", "Espera prom.", "Retorno prom.", "Cambios contexto");
    printf("%-12s %12.2f %14.2f %20s\n", "FCFS", esp_fcfs, ret_fcfs, "-");
    printf("%-12s %12.2f %14.2f %20s\n", "SJF", esp_sjf, ret_sjf, "-");
    printf("%-12s %12.2f %14.2f %20d\n", "RR (q=2)", esp_rr2, ret_rr2, cc_rr2);
    printf("%-12s %12.2f %14.2f %20d\n", "RR (q=4)", esp_rr4, ret_rr4, cc_rr4);
    printf("%-12s %12.2f %14.2f %20s\n", "Prioridades", esp_prio, ret_prio, "-");

    return 0;
}
```

**Ejecución** (`./bin/comparativa`, dataset 5.1):

```text
=== FCFS ===

PID  AT  BT Prio Inicio Fin Espera Retorno
  1   0   5    2      0   5      0       5
  2   1   3    1      5   8      4       7
  3   2   1    3      8   9      6       7
  4   3   2    1      9  11      6       8
  5   4   4    2     11  15      7      11
Promedio espera : 4.60
Promedio retorno: 7.60

Diagrama de Gantt:
| P1 | P2 | P3 | P4 | P5 |
0    5    8    9    11   15

=== SJF (no apropiativo) ===

PID  AT  BT Prio Inicio Fin Espera Retorno
  1   0   5    2      0   5      0       5
  2   1   3    1      8  11      7      10
  3   2   1    3      5   6      3       4
  4   3   2    1      6   8      3       5
  5   4   4    2     11  15      7      11
Promedio espera : 4.00
Promedio retorno: 7.00

Diagrama de Gantt:
| P1 | P3 | P4 | P2 | P5 |
0    5    6    8    11   15

=== Round Robin (quantum = 2) ===

PID  AT  BT Prio Inicio Fin Espera Retorno
  1   0   5    2      0  13      8      13
  2   1   3    1      2  12      8      11
  3   2   1    3      4   5      2       3
  4   3   2    1      7   9      4       6
  5   4   4    2      9  15      7      11
Promedio espera : 5.80
Promedio retorno: 8.80

Diagrama de Gantt:
| P1 | P2 | P3 | P1 | P4 | P5 | P2 | P1 | P5 |
0    2    4    5    7    9    11   12   13   15

Cambios de contexto: 8

=== Round Robin (quantum = 4) ===

PID  AT  BT Prio Inicio Fin Espera Retorno
  1   0   5    2      0  15     10      15
  2   1   3    1      4   7      3       6
  3   2   1    3      7   8      5       6
  4   3   2    1      8  10      5       7
  5   4   4    2     10  14      6      10
Promedio espera : 5.80
Promedio retorno: 8.80

Diagrama de Gantt:
| P1 | P2 | P3 | P4 | P5 | P1 |
0    4    7    8    10   14   15

Cambios de contexto: 5

=== Prioridades (no apropiativo) ===

PID  AT  BT Prio Inicio Fin Espera Retorno
  1   0   5    2      0   5      0       5
  2   1   3    1      5   8      4       7
  3   2   1    3     14  15     12      13
  4   3   2    1      8  10      5       7
  5   4   4    2     10  14      6      10
Promedio espera : 5.40
Promedio retorno: 8.40

Diagrama de Gantt:
| P1 | P2 | P4 | P5 | P3 |
0    5    8    10   14   15

=== Tabla comparativa ===

Algoritmo    Espera prom.  Retorno prom.     Cambios contexto
FCFS                 4.60           7.60                    -
SJF                  4.00           7.00                    -
RR (q=2)             5.80           8.80                    8
RR (q=4)             5.80           8.80                    5
Prioridades          5.40           8.40                    -
```

[CAPTURA: s2_comparativa_exec.png]

**Lectura de la tabla comparativa:** sobre este dataset, SJF logra la
menor espera/retorno promedio porque minimiza el tiempo que los procesos
cortos pasan en cola; FCFS queda en segundo lugar; RR y Prioridades quedan
por encima porque introducen apropiación (RR) o dejan procesos cortos de
baja prioridad al final (P3 en Prioridades). RR con quantum mayor (q=4)
reduce los cambios de contexto de 8 a 5 sin cambiar los promedios en este
caso, porque ningún proceso individual excede 2 quantums de 4.

### 5.5 Ejercicios propuestos — Sesión 2 (6.1 a 6.4)

#### 6.1 — Aging (envejecimiento de prioridades) — documentación

Ver sección **"6.1 — Aging"** dentro de las respuestas de documentación al
final de este informe (sección 9.1).

#### 6.2 — `ejercicios/s2_62_rr_prio.c`: Round Robin apropiativo con prioridades

Implementa **multinivel por colas FIFO** (niveles 1 = mayor prioridad a 5 =
menor). Cada unidad de tiempo: (1) encola los procesos que llegan en `t` y,
si alguno tiene mayor prioridad que el proceso en ejecución, lo desaloja
(vuelve al final de su cola); (2) si la CPU está libre, despacha el frente
de la cola de mayor prioridad no vacía con un quantum completo; (3) ejecuta
una unidad; si termina, se completa, si agota el quantum sin terminar, se
desaloja.

Dataset (quantum = 2), diseñado para mostrar RR dentro de un mismo nivel
(P1/P5, prioridad 2) y desalojo por llegada de mayor prioridad (P2,
prioridad 1, llega en `t=1` y desaloja a P1):

| PID | AT | BT | Prioridad |
|-----|----|----|-----------|
| P1 | 0 | 5 | 2 |
| P2 | 1 | 3 | 1 |
| P3 | 2 | 4 | 3 |
| P4 | 3 | 2 | 1 |
| P5 | 0 | 3 | 2 |

**Ejecución** (`./bin/s2_62_rr_prio`):

```text
=== Dataset (quantum = 2, prioridad 1 = mayor) ===

PID  AT  BT Prio
  1   0   5    2
  2   1   3    1
  3   2   4    3
  4   3   2    1
  5   0   3    2

=== RR apropiativo con prioridades ===

PID  AT  BT Prio Inicio Fin Espera Retorno
  1   0   5    2      0  13      8      13
  2   1   3    1      1   4      0       3
  3   2   4    3     13  17     11      15
  4   3   2    1      4   6      1       3
  5   0   3    2      6  11      8      11
Promedio espera : 5.60
Promedio retorno: 9.00

Diagrama de Gantt:
| P1 | P2 | P4 | P5 | P1 | P5 | P1 | P3 |
0    1    4    6    8    10   11   13   17

Cambios de contexto: 7
```

[CAPTURA: ej_6_2_exec.png]

**Traza:** P1 empieza en `t=0`. En `t=1` llega P2 (prioridad 1 < 2), desaloja
a P1 (que vuelve a la cola de nivel 2) y P2 corre `[1,4)` sin interrupción
(BT=3 < quantum). En `t=4`, nivel 1 tiene a P4 (llegó en `t=3`) → P4 corre
`[4,6)`. Libre el CPU, nivel 2 tiene a P5 y P1 (en ese orden) → P5 corre
`[6,8)`, agota quantum, vuelve a la cola; P1 corre `[8,10)`, agota quantum;
P5 corre `[10,11)` y termina (BT=3 cumplido); P1 corre `[11,13)` y termina
(BT=5 cumplido). Finalmente P3 (único proceso de nivel 3) corre `[13,17)`.
Total: **7 cambios de contexto**, todos coinciden con el hand-trace previo a
la compilación.

#### 6.3 — `ejercicios/s2_63_quantums.c`: RR con q=1, q=3, q=100 vs FCFS

Mismo dataset de la sección 5.1 (sin prioridades). Corre RR con tres
quantums distintos y FCFS, para mostrar que un quantum muy grande (q=100,
mayor que cualquier ráfaga) hace que **ningún proceso agote su quantum**, por
lo que RR no desaloja a nadie y el resultado (inicio/fin/espera/retorno por
proceso) coincide exactamente con FCFS.

**Ejecución** (`./bin/s2_63_quantums`):

```text
=== Dataset ===

PID  AT  BT
  1   0   5
  2   1   3
  3   2   1
  4   3   2
  5   4   4

=== Round Robin (q=1) ===

PID  AT  BT Inicio Fin Espera Retorno
  1   0   5      0  13      8      13
  2   1   3      1   9      5       8
  3   2   1      3   4      1       2
  4   3   2      5  10      5       7
  5   4   4      7  15      7      11
Promedio espera : 5.20
Promedio retorno: 8.20

Diagrama de Gantt:
| P1 | P2 | P1 | P3 | P2 | P4 | P1 | P5 | P2 | P4 | P1 | P5 | P1 | P5 |
0    1    2    3    4    5    6    7    8    9    10   11   12   13   15

Cambios de contexto: 13

=== Round Robin (q=3) ===

PID  AT  BT Inicio Fin Espera Retorno
  1   0   5      0  11      6      11
  2   1   3      3   6      2       5
  3   2   1      6   7      4       5
  4   3   2      7   9      4       6
  5   4   4     11  15      7      11
Promedio espera : 4.60
Promedio retorno: 7.60

Diagrama de Gantt:
| P1 | P2 | P3 | P4 | P1 | P5 |
0    3    6    7    9    11   15

Cambios de contexto: 5

=== Round Robin (q=100) ===

PID  AT  BT Inicio Fin Espera Retorno
  1   0   5      0   5      0       5
  2   1   3      5   8      4       7
  3   2   1      8   9      6       7
  4   3   2      9  11      6       8
  5   4   4     11  15      7      11
Promedio espera : 4.60
Promedio retorno: 7.60

Diagrama de Gantt:
| P1 | P2 | P3 | P4 | P5 |
0    5    8    9    11   15

Cambios de contexto: 4

=== FCFS ===

PID  AT  BT Inicio Fin Espera Retorno
  1   0   5      0   5      0       5
  2   1   3      5   8      4       7
  3   2   1      8   9      6       7
  4   3   2      9  11      6       8
  5   4   4     11  15      7      11
Promedio espera : 4.60
Promedio retorno: 7.60

Diagrama de Gantt:
| P1 | P2 | P3 | P4 | P5 |
0    5    8    9    11   15

=== Tabla comparativa ===

Algoritmo    Espera prom.  Retorno prom.   Cambios contexto
RR (q=1)             5.20           8.20                 13
RR (q=3)             4.60           7.60                  5
RR (q=100)           4.60           7.60                  4
FCFS                 4.60           7.60                  -

Conclusión: con q=100 (mayor que cualquier ráfaga), ningún
proceso agota su quantum antes de terminar, por lo que no hay
desalojos por RR. Los 4 cambios de contexto restantes son solo
los pases naturales de un proceso a otro al completarse (igual
que ocurriría en FCFS). Inicio/fin/espera/retorno por proceso son
IDÉNTICOS a los de FCFS.
```

[CAPTURA: ej_6_3_exec.png]

A menor quantum (q=1), la espera promedio empeora (5.20 vs 4.60) porque
aumentan drásticamente los cambios de contexto (13), repartiendo la CPU en
porciones tan pequeñas que ningún proceso avanza mucho por turno. Con q=3 y
q=100 los promedios ya coinciden con FCFS; q=100 además iguala
**proceso a proceso** los tiempos de inicio/fin con FCFS, confirmando que
`q -> infinito` (o `q >= max(BT)`) hace que **RR converja a FCFS**.

#### 6.4 — Recomendación interactivo vs. batch — documentación

Ver sección **"6.4 — Interactivo vs. batch"** en la sección 9.2 de este
informe.

**Resumen Sesión 2 — ejercicios:** 4/4 resueltos (6.1 doc, 6.2, 6.3, 6.4 doc).

---

## 6. Sesión 3 — Hilos y sincronización

### 6.1 Actividad A1 — Hilos básicos (`sesion3-hilos/hilos_basico.c`)

```c
/*
 * hilos_basico.c
 * Sesión 3 — Actividad A1: hilos básicos con pthreads.
 *
 * Lanza 4 hilos; cada uno recibe su ID por puntero a memoria propia
 * (un elemento distinto de un arreglo, nunca `&i` de un bucle) e
 * imprime "Hilo i iniciado" / "Hilo i finalizado". El hilo principal
 * espera a todos con pthread_join.
 *
 * Compilar: make s3
 *   (equivalente a: gcc -Wall -Wextra -std=c11 -pthread
 *    sesion3-hilos/hilos_basico.c -o bin/hilos_basico)
 */

#include <pthread.h>
#include <stdio.h>

#define NUM_HILOS 4

static void *tarea(void *arg) {
    int id = *(int *) arg;

    printf("Hilo %d iniciado\n", id);
    printf("Hilo %d finalizado\n", id);

    return NULL;
}

int main(void) {
    pthread_t hilos[NUM_HILOS];
    int ids[NUM_HILOS];

    for (int i = 0; i < NUM_HILOS; i++) {
        ids[i] = i + 1;
        pthread_create(&hilos[i], NULL, tarea, &ids[i]);
    }

    for (int i = 0; i < NUM_HILOS; i++) {
        pthread_join(hilos[i], NULL);
    }

    return 0;
}
```

**Ejecución** (`./bin/hilos_basico`):

```text
Hilo 1 iniciado
Hilo 1 finalizado
Hilo 2 iniciado
Hilo 2 finalizado
Hilo 3 iniciado
Hilo 3 finalizado
Hilo 4 iniciado
Hilo 4 finalizado
```

[CAPTURA: s3_hilos_basico_exec.png]

Cada hilo recibe su identificador a través de `&ids[i]` (memoria propia
dentro de un arreglo), nunca `&i` de la variable del bucle — si se pasara
`&i`, varios hilos podrían leer el mismo valor de `i` (ya modificado por el
bucle) antes de copiarlo, produciendo IDs repetidos o incorrectos.

### 6.2 Actividad A2 — Condición de carrera (`sesion3-hilos/race_condition.c`)

```c
/*
 * race_condition.c
 * Sesión 3 — Actividad A2: condición de carrera y corrección con mutex.
 *
 * Lanza 4 hilos que incrementan 100 000 veces un contador global
 * (esperado 400 000). Sin protección, `contador++` no es atómico
 * (lee -> incrementa -> escribe) y dos hilos pueden pisarse, dando un
 * resultado final menor al esperado y distinto en cada ejecución. Con
 * `pthread_mutex_lock/unlock` la sección crítica queda protegida y el
 * resultado siempre es 400000.
 *
 * Uso:
 *   ./race_condition          -> sin mutex (demuestra la carrera)
 *   ./race_condition mutex    -> con mutex (resultado correcto)
 *
 * Compilar: make s3
 *   (equivalente a: gcc -Wall -Wextra -std=c11 -pthread
 *    sesion3-hilos/race_condition.c -o bin/race_condition)
 */

#include <pthread.h>
#include <stdio.h>
#include <string.h>

#define NUM_HILOS 4
#define INCREMENTOS 100000

static long contador = 0;
static int usar_mutex = 0;
static pthread_mutex_t mutex_contador = PTHREAD_MUTEX_INITIALIZER;

static void *incrementar(void *arg) {
    (void) arg;

    for (int i = 0; i < INCREMENTOS; i++) {
        if (usar_mutex) {
            pthread_mutex_lock(&mutex_contador);
            contador++;
            pthread_mutex_unlock(&mutex_contador);
        } else {
            contador++;
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    usar_mutex = (argc > 1 && strcmp(argv[1], "mutex") == 0);

    pthread_t hilos[NUM_HILOS];

    for (int i = 0; i < NUM_HILOS; i++) {
        pthread_create(&hilos[i], NULL, incrementar, NULL);
    }

    for (int i = 0; i < NUM_HILOS; i++) {
        pthread_join(hilos[i], NULL);
    }

    printf("Modo: %s\n", usar_mutex ? "con mutex" : "sin mutex");
    printf("Contador final: %ld (esperado %d)\n",
           contador, NUM_HILOS * INCREMENTOS);

    return 0;
}
```

**Evidencia de la condición de carrera — sin mutex (3 corridas, `./bin/race_condition`):**

```text
Modo: sin mutex
Contador final: 127843 (esperado 400000)

Modo: sin mutex
Contador final: 301450 (esperado 400000)

Modo: sin mutex
Contador final: 308451 (esperado 400000)
```

[CAPTURA: s3_race_sin_mutex.png]

**Corrección con mutex (3 corridas, `./bin/race_condition mutex`):**

```text
Modo: con mutex
Contador final: 400000 (esperado 400000)

Modo: con mutex
Contador final: 400000 (esperado 400000)

Modo: con mutex
Contador final: 400000 (esperado 400000)
```

[CAPTURA: s3_race_con_mutex.png]

**Análisis:** sin mutex, cada corrida da un resultado **distinto y siempre
menor** a 400 000 (en nuestras 3 corridas: 127843, 301450, 308451). Esto
ocurre porque `contador++` se traduce en al menos tres pasos máquina (leer,
incrementar, escribir) que no son atómicos: dos hilos pueden leer el mismo
valor antes de que ninguno escriba el resultado incrementado, y uno de los
dos incrementos se "pierde". Con `pthread_mutex_lock/unlock` alrededor de
`contador++`, solo un hilo a la vez puede leer-modificar-escribir, y las
3 corridas dan exactamente 400 000 — el resultado correcto y reproducible.

### 6.3 Actividad A3 — Productor-Consumidor (`sesion3-hilos/productor_consumidor.c`)

```c
/*
 * productor_consumidor.c
 * Sesión 3 — Actividad A3: productor-consumidor con buffer circular.
 *
 * 1 productor genera 20 números aleatorios (1-100) y los coloca en un
 * buffer circular de tamaño 5 (espera si está lleno); 1 consumidor
 * retira los 20 (espera si está vacío). Sincronización con
 * sem_t vacios = 5, llenos = 0 y un mutex para el acceso al buffer.
 * Cada inserción/extracción se registra junto con su posición en el
 * buffer.
 *
 * Orden de operaciones (evita el deadlock mutex/semáforo invertidos):
 *   productor:  sem_wait(vacios) -> lock -> ... -> unlock -> sem_post(llenos)
 *   consumidor: sem_wait(llenos) -> lock -> ... -> unlock -> sem_post(vacios)
 *
 * Uso: ./productor_consumidor [--seed N]
 *
 * Compilar: make s3
 *   (equivalente a: gcc -Wall -Wextra -std=c11 -pthread
 *    sesion3-hilos/productor_consumidor.c -o bin/productor_consumidor)
 */

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TAM_BUFFER 5
#define NUM_ITEMS 20

static int buffer[TAM_BUFFER];
static int pos_in = 0;
static int pos_out = 0;

static sem_t vacios;
static sem_t llenos;
static pthread_mutex_t mutex_buffer = PTHREAD_MUTEX_INITIALIZER;

static void *productor(void *arg) {
    (void) arg;

    for (int i = 0; i < NUM_ITEMS; i++) {
        int valor = 1 + rand() % 100;

        sem_wait(&vacios);
        pthread_mutex_lock(&mutex_buffer);

        buffer[pos_in] = valor;
        printf("Productor  -> coloca %3d en posición %d\n", valor, pos_in);
        pos_in = (pos_in + 1) % TAM_BUFFER;

        pthread_mutex_unlock(&mutex_buffer);
        sem_post(&llenos);
    }

    return NULL;
}

static void *consumidor(void *arg) {
    (void) arg;

    for (int i = 0; i < NUM_ITEMS; i++) {
        sem_wait(&llenos);
        pthread_mutex_lock(&mutex_buffer);

        int valor = buffer[pos_out];
        printf("Consumidor -> retira %3d de posición %d\n", valor, pos_out);
        pos_out = (pos_out + 1) % TAM_BUFFER;

        pthread_mutex_unlock(&mutex_buffer);
        sem_post(&vacios);
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    unsigned int semilla = (unsigned int) time(NULL);

    if (argc == 3 && strcmp(argv[1], "--seed") == 0) {
        semilla = (unsigned int) atoi(argv[2]);
    }
    srand(semilla);

    sem_init(&vacios, 0, TAM_BUFFER);
    sem_init(&llenos, 0, 0);

    pthread_t hilo_productor;
    pthread_t hilo_consumidor;

    pthread_create(&hilo_productor, NULL, productor, NULL);
    pthread_create(&hilo_consumidor, NULL, consumidor, NULL);

    pthread_join(hilo_productor, NULL);
    pthread_join(hilo_consumidor, NULL);

    sem_destroy(&vacios);
    sem_destroy(&llenos);

    return 0;
}
```

**Ejecución** (`./bin/productor_consumidor --seed 1`):

```text
Productor  -> coloca  84 en posición 0
Productor  -> coloca  87 en posición 1
Productor  -> coloca  78 en posición 2
Productor  -> coloca  16 en posición 3
Productor  -> coloca  94 en posición 4
Consumidor -> retira  84 de posición 0
Consumidor -> retira  87 de posición 1
Consumidor -> retira  78 de posición 2
Consumidor -> retira  16 de posición 3
Consumidor -> retira  94 de posición 4
Productor  -> coloca  36 en posición 0
Productor  -> coloca  87 en posición 1
Productor  -> coloca  93 en posición 2
Productor  -> coloca  50 en posición 3
Productor  -> coloca  22 en posición 4
Consumidor -> retira  36 de posición 0
Consumidor -> retira  87 de posición 1
Consumidor -> retira  93 de posición 2
Consumidor -> retira  50 de posición 3
Consumidor -> retira  22 de posición 4
Productor  -> coloca  63 en posición 0
Consumidor -> retira  63 de posición 0
Productor  -> coloca  28 en posición 1
Productor  -> coloca  91 en posición 2
Productor  -> coloca  60 en posición 3
Productor  -> coloca  64 en posición 4
Productor  -> coloca  27 en posición 0
Consumidor -> retira  28 de posición 1
Consumidor -> retira  91 de posición 2
Consumidor -> retira  60 de posición 3
Consumidor -> retira  64 de posición 4
Productor  -> coloca  41 en posición 1
Consumidor -> retira  27 de posición 0
Productor  -> coloca  27 en posición 2
Consumidor -> retira  41 de posición 1
Productor  -> coloca  73 en posición 3
Consumidor -> retira  27 de posición 2
Productor  -> coloca  37 en posición 4
Consumidor -> retira  73 de posición 3
Consumidor -> retira  37 de posición 4
```

[CAPTURA: s3_productor_consumidor_exec.png]

Cada valor se "coloca" en una posición y, en algún momento posterior, se
"retira" de la **misma posición** con el **mismo valor**, y nunca hay más
de `TAM_BUFFER = 5` elementos pendientes de consumir — evidencia de que el
par de semáforos contadores (`vacios`/`llenos`) más el mutex mantienen la
exclusión mutua y el balance de huecos/elementos del buffer circular sin
condiciones de carrera.

### 6.4 Ejercicios propuestos — Sesión 3 (7.1 a 7.4)

#### 7.1 — `ejercicios/s3_71_2p2c.c`: Productor-Consumidor 2P/2C

Extiende la actividad A3 a **2 productores y 2 consumidores** sobre el
mismo buffer circular de tamaño 5, manteniendo el mismo orden de
operaciones (`sem_wait(vacios) -> lock -> ... -> unlock -> sem_post(llenos)`
y simétrico en consumo). Cada hilo recibe su ID por puntero a memoria propia
(`ids_productores[i]` / `ids_consumidores[i]`), nunca `&i`. Cada productor
genera 10 ítems y cada consumidor retira 10 (20 producidos = 20 consumidos).

**Ejecución** (`./bin/s3_71_2p2c --seed 7`):

```text
=== Productor-Consumidor 2P/2C (buffer de 5, 10 ítems por hilo) ===

Productor 2 -> coloca  78 en posición 0
Productor 2 -> coloca 100 en posición 1
Productor 2 -> coloca 100 en posición 2
Productor 2 -> coloca  72 en posición 3
Productor 2 -> coloca  26 en posición 4
Consumidor 1 -> retira  78 de posición 0
Consumidor 2 -> retira 100 de posición 1
Consumidor 1 -> retira 100 de posición 2
Consumidor 2 -> retira  72 de posición 3
Consumidor 1 -> retira  26 de posición 4
Productor 1 -> coloca  87 en posición 0
Productor 1 -> coloca  98 en posición 1
Productor 1 -> coloca   1 en posición 2
Productor 1 -> coloca  54 en posición 3
Productor 1 -> coloca  16 en posición 4
Consumidor 1 -> retira  87 de posición 0
Consumidor 1 -> retira  98 de posición 1
Consumidor 1 -> retira   1 de posición 2
Consumidor 1 -> retira  54 de posición 3
Consumidor 1 -> retira  16 de posición 4
Productor 2 -> coloca  44 en posición 0
Productor 1 -> coloca  22 en posición 1
Productor 1 -> coloca  32 en posición 2
Productor 1 -> coloca  27 en posición 3
Productor 1 -> coloca  66 en posición 4
Consumidor 1 -> retira  44 de posición 0
Productor 2 -> coloca  79 en posición 0
Consumidor 1 -> retira  22 de posición 1
Productor 1 -> coloca  21 en posición 1
Consumidor 2 -> retira  32 de posición 2
Consumidor 2 -> retira  27 de posición 3
Consumidor 2 -> retira  66 de posición 4
Consumidor 2 -> retira  79 de posición 0
Consumidor 2 -> retira  21 de posición 1
Productor 2 -> coloca  47 en posición 2
Productor 2 -> coloca  71 en posición 3
Productor 2 -> coloca  37 en posición 4
Consumidor 2 -> retira  47 de posición 2
Consumidor 2 -> retira  71 de posición 3
Consumidor 2 -> retira  37 de posición 4

Total producido : 20
Total consumido : 20
```

[CAPTURA: ej_7_1_exec.png]

Se corrió 3 veces con semillas distintas (1, 2, 3) y en todas el total
producido y consumido coincide en 20/20, sin que el programa se quede
bloqueado (deadlock) ni el buffer reciba más de 5 elementos simultáneos.

#### 7.2 — `ejercicios/s3_72_tiempo.c`: Medición de tiempo con/sin mutex

Variante de la actividad A2 que mide con `clock_gettime(CLOCK_MONOTONIC)`
el tiempo de pared de 4 hilos incrementando un contador
`NUM_HILOS * INCREMENTOS = 4 000 000` veces, sin y con mutex.

**Ejecución** (`./bin/s3_72_tiempo`):

```text
=== Medición de tiempo: 4 hilos x 1000000 incrementos ===

Modo             Contador     Esperado Tiempo (s)
sin mutex         1404522      4000000     0.0149  <-- condición de carrera
con mutex         4000000      4000000     0.2800

Conclusión: sin mutex el contador final suele ser menor que el
esperado (incrementos perdidos por la condición de carrera) y la
ejecución es más rápida porque no hay espera por el lock. Con
mutex el resultado es siempre correcto (4000000), a costa de un
tiempo mayor por la sincronización de la sección crítica.
Diferencia de tiempo (con - sin) = 0.2650 s
```

[CAPTURA: ej_7_2_exec.png]

La versión sin mutex es ~19x más rápida (0.0149 s vs 0.2800 s), pero pierde
más del 64% de los incrementos (1 404 522 de 4 000 000). El costo de la
sincronización (~0.265 s) es el precio de la corrección: cada
`lock`/`unlock` implica una operación atómica y, potencialmente, hacer ceder
el procesador a otro hilo.

#### 7.3 — Mutex vs. semáforo — documentación

Ver sección **"7.3 — Mutex vs. semáforo"** en la sección 9.3 de este informe.

#### 7.4 — `ejercicios/s3_74_suma_hilos.c`: Suma paralela de un arreglo (fork → hilos)

Suma un arreglo de 1 000 000 elementos dividiéndolo en `NUM_HILOS = 4`
bloques contiguos; cada hilo recibe por puntero (a memoria propia, un
`BloqueSuma` por hilo) el rango `[inicio, fin)` que debe sumar y escribe su
resultado en `b->suma`. El hilo principal suma las 4 parciales tras los
`pthread_join` y compara contra la suma secuencial.

**Ejecución** (`./bin/s3_74_suma_hilos`):

```text
=== Suma paralela de un arreglo de 1000000 elementos (4 hilos) ===

Hilo 0 -> rango [0, 250000) -> suma parcial = 31250125000
Hilo 1 -> rango [250000, 500000) -> suma parcial = 93750125000
Hilo 2 -> rango [500000, 750000) -> suma parcial = 156250125000
Hilo 3 -> rango [750000, 1000000) -> suma parcial = 218750125000

Modo                    Suma   Tiempo (s)
secuencial      500000500000     0.002391
con hilos       500000500000     0.001420

Las sumas coinciden: 500000500000

fork vs hilos: con fork() cada proceso hijo trabajaría sobre
una COPIA del arreglo y su suma parcial viviría en su propia
memoria, por lo que el padre necesitaría IPC (pipe, memoria
compartida) para recolectar los resultados. Con hilos, todos
comparten el arreglo y escriben su suma parcial en
`bloques[i].suma`, así que combinarlas es solo un bucle tras
los `pthread_join`, sin IPC ni copias.
```

[CAPTURA: ej_7_4_exec.png]

La suma con hilos coincide exactamente con la secuencial
(`500000500000`) y, en esta corrida, es ~1.7x más rápida
(0.001420 s vs 0.002391 s) gracias al paralelismo en 4 núcleos.

**Resumen Sesión 3 — ejercicios:** 4/4 resueltos (7.1, 7.2, 7.3 doc, 7.4).

---

## 7. Sesión 4 — Proyecto integrador: simulador Banco/Caja

### 7.1 Escenario elegido

Se modela una **agencia bancaria** con `M` ventanillas (capacidad limitada) y
`N` clientes que llegan en instantes aleatorios y requieren un tiempo de
atención aleatorio. Cada cliente es un hilo; las ventanillas son un **recurso
compartido** representado por un semáforo contador. El programa admite un
modo `--prioridad` en el que los clientes marcados como "adulto mayor" pasan
primero (manteniendo FCFS dentro de cada grupo de prioridad).

### 7.2 Diseño de sincronización

| Mecanismo | Variable | Protege / representa |
|---|---|---|
| Semáforo contador | `sem_t recurso` (inicializado en `capacidad`) | Cupos disponibles en las `M` ventanillas |
| Mutex | `pthread_mutex_t mutex_estado` | Contador `ocupado`, la cola `cola[]` y la escritura del log (consola + `log_simulacion.txt`) |
| Variable de condición | `pthread_cond_t cv_turno` | Señala a los clientes en espera cuando cambia el estado (alguien sale o llega) para reevaluar si "es su turno" |

Cada cliente sigue la secuencia: `sleep(llegada)` → `lock(mutex_estado)` →
encolarse y registrar "LLEGA" → `while (!puede_pasar) cond_wait(cv_turno)` →
desencolarse → `sem_wait(recurso)` → `ocupado++` → registrar "INGRESA" →
`unlock(mutex_estado)` → `sleep(rafaga)` → `lock(mutex_estado)` →
`ocupado--` → `sem_post(recurso)` → registrar "SALE" →
`cond_broadcast(cv_turno)` → `unlock(mutex_estado)`.

`puede_pasar(idx)` exige dos condiciones: que ningún otro cliente en la cola
deba atenderse antes (`es_su_turno`, que en modo `--prioridad` da preferencia
a los adultos mayores y, si no, usa orden de llegada/PID) **y** que
`ocupado < capacidad`.

### 7.3 Código fuente (`sesion4-integrador/simulador.c`)

```c
/*
 * simulador.c
 * Sesión 4 — Proyecto integrador: escenario Banco/Caja.
 *
 * M ventanillas modeladas con un semáforo contador (sem_t recurso,
 * inicializado en `capacidad`). N clientes (hilos) llegan en instantes
 * aleatorios y requieren un tiempo de atención (ráfaga) aleatorio.
 * Planificación FCFS por defecto; con --prioridad, los clientes marcados
 * "adulto mayor" pasan primero (dentro de los que esperan turno),
 * manteniendo FCFS entre clientes de igual prioridad.
 *
 * Cada cliente: sem_wait(&recurso) -> usa el recurso (sleep = ráfaga) ->
 * sem_post(&recurso). El contador `ocupado` y la escritura del log
 * (consola + log_simulacion.txt) están protegidos por mutex_estado.
 *
 * Uso: ./simulador [n_clientes] [capacidad] [--prioridad] [--seed N]
 *   n_clientes (por defecto 6), capacidad = ventanillas (por defecto 3)
 *
 * Compilar: make s4
 *   (equivalente a: gcc -Wall -Wextra -std=c11 -Icommon
 *    sesion4-integrador/simulador.c common/gantt.c common/planificador.c
 *    common/proceso.c -o bin/simulador -pthread)
 */

#include <pthread.h>
#include <semaphore.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define MAX_CLIENTES 100

typedef struct {
    int id;
    int llegada;
    int rafaga;
    int adulto_mayor;
} Cliente;

static Cliente clientes[MAX_CLIENTES];
static int n_clientes;
static int capacidad;
static int modo_prioridad = 0;

static sem_t recurso;
static pthread_mutex_t mutex_estado = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cv_turno = PTHREAD_COND_INITIALIZER;

static int cola[MAX_CLIENTES];
static int tam_cola = 0;
static int ocupado = 0;

static double suma_espera = 0.0;
static struct timeval t0;
static FILE *log_archivo;

/* Segundos transcurridos desde t0. */
static double tiempo_transcurrido(void) {
    struct timeval ahora;

    gettimeofday(&ahora, NULL);
    return (ahora.tv_sec - t0.tv_sec) + (ahora.tv_usec - t0.tv_usec) / 1e6;
}

/* Instante lógico (segundos enteros redondeados) para los logs. */
static int t_actual(void) {
    return (int) (tiempo_transcurrido() + 0.5);
}

/* Escribe un evento "[t=T] ..." en consola y en el log. El llamador
 * debe tener tomado mutex_estado. */
static void registrar(int t, const char *formato, ...) {
    va_list args;

    printf("[t=%d] ", t);
    va_start(args, formato);
    vprintf(formato, args);
    va_end(args);
    printf("\n");

    fprintf(log_archivo, "[t=%d] ", t);
    va_start(args, formato);
    vfprintf(log_archivo, formato, args);
    va_end(args);
    fprintf(log_archivo, "\n");
}

/* <0 si el cliente a debe atenderse antes que el cliente b. */
static int comparar_prioridad(int a, int b) {
    const Cliente *ca = &clientes[a];
    const Cliente *cb = &clientes[b];

    if (modo_prioridad && ca->adulto_mayor != cb->adulto_mayor) {
        return cb->adulto_mayor - ca->adulto_mayor; /* adulto mayor primero */
    }
    if (ca->llegada != cb->llegada) {
        return ca->llegada - cb->llegada;
    }
    return ca->id - cb->id;
}

/* true si ningún otro cliente en cola debe atenderse antes que idx. */
static int es_su_turno(int idx) {
    for (int k = 0; k < tam_cola; k++) {
        if (cola[k] != idx && comparar_prioridad(cola[k], idx) < 0) {
            return 0;
        }
    }
    return 1;
}

/* true si idx puede pasar ya: es su turno por prioridad y hay un
 * lugar libre en el recurso. */
static int puede_pasar(int idx) {
    return es_su_turno(idx) && ocupado < capacidad;
}

static void *atender_cliente(void *arg) {
    int idx = *(int *) arg;
    Cliente *c = &clientes[idx];

    sleep((unsigned) c->llegada);

    pthread_mutex_lock(&mutex_estado);

    int t_llegada = t_actual();
    cola[tam_cola++] = idx;

    if (puede_pasar(idx)) {
        registrar(t_llegada, "Cliente %d LLEGA y solicita recurso", c->id);
    } else {
        registrar(t_llegada, "Cliente %d LLEGA y solicita recurso (en espera)", c->id);
    }

    while (!puede_pasar(idx)) {
        pthread_cond_wait(&cv_turno, &mutex_estado);
    }
    for (int k = 0; k < tam_cola; k++) {
        if (cola[k] == idx) {
            for (int j = k; j < tam_cola - 1; j++) {
                cola[j] = cola[j + 1];
            }
            tam_cola--;
            break;
        }
    }

    /* ocupado < capacidad ya verificado: sem_wait no bloquea. */
    sem_wait(&recurso);
    ocupado++;
    int t_ingreso = t_actual();
    suma_espera += (t_ingreso - t_llegada);
    registrar(t_ingreso, "Cliente %d INGRESA (recurso ocupado: %d/%d)", c->id, ocupado, capacidad);

    pthread_mutex_unlock(&mutex_estado);

    sleep((unsigned) c->rafaga);

    pthread_mutex_lock(&mutex_estado);
    ocupado--;
    sem_post(&recurso);
    registrar(t_actual(), "Cliente %d SALE (recurso ocupado: %d/%d)", c->id, ocupado, capacidad);
    pthread_cond_broadcast(&cv_turno);
    pthread_mutex_unlock(&mutex_estado);

    return NULL;
}

int main(int argc, char *argv[]) {
    n_clientes = 6;
    capacidad = 3;
    unsigned int semilla = (unsigned int) time(NULL);

    int argi = 1;

    if (argi < argc && argv[argi][0] != '-') {
        n_clientes = atoi(argv[argi++]);
    }
    if (argi < argc && argv[argi][0] != '-') {
        capacidad = atoi(argv[argi++]);
    }
    for (; argi < argc; argi++) {
        if (strcmp(argv[argi], "--prioridad") == 0) {
            modo_prioridad = 1;
        } else if (strcmp(argv[argi], "--seed") == 0 && argi + 1 < argc) {
            semilla = (unsigned int) atoi(argv[++argi]);
        }
    }

    if (n_clientes < 1) {
        n_clientes = 1;
    }
    if (n_clientes > MAX_CLIENTES) {
        n_clientes = MAX_CLIENTES;
    }
    if (capacidad < 1) {
        capacidad = 1;
    }

    srand(semilla);

    for (int i = 0; i < n_clientes; i++) {
        clientes[i].id = i + 1;
        clientes[i].llegada = rand() % n_clientes;
        clientes[i].rafaga = 1 + rand() % 3;
        clientes[i].adulto_mayor = (rand() % 100) < 30;
    }

    log_archivo = fopen("log_simulacion.txt", "w");
    if (log_archivo == NULL) {
        perror("fopen");
        return 1;
    }

    printf("=== Simulación Banco/Caja ===\n");
    printf("Clientes: %d | Ventanillas: %d | Modo: %s | Semilla: %u\n",
           n_clientes, capacidad,
           modo_prioridad ? "FCFS + prioridad adulto mayor" : "FCFS",
           semilla);
    fprintf(log_archivo, "=== Simulación Banco/Caja ===\n");
    fprintf(log_archivo, "Clientes: %d | Ventanillas: %d | Modo: %s | Semilla: %u\n",
            n_clientes, capacidad,
            modo_prioridad ? "FCFS + prioridad adulto mayor" : "FCFS",
            semilla);

    for (int i = 0; i < n_clientes; i++) {
        printf("  Cliente %d: llegada=%d  atencion=%d%s\n",
               clientes[i].id, clientes[i].llegada, clientes[i].rafaga,
               clientes[i].adulto_mayor ? "  (adulto mayor)" : "");
        fprintf(log_archivo, "  Cliente %d: llegada=%d  atencion=%d%s\n",
                clientes[i].id, clientes[i].llegada, clientes[i].rafaga,
                clientes[i].adulto_mayor ? "  (adulto mayor)" : "");
    }
    printf("\n");
    fprintf(log_archivo, "\n");

    sem_init(&recurso, 0, (unsigned) capacidad);
    gettimeofday(&t0, NULL);

    pthread_t hilos[MAX_CLIENTES];
    int ids[MAX_CLIENTES];

    for (int i = 0; i < n_clientes; i++) {
        ids[i] = i;
        pthread_create(&hilos[i], NULL, atender_cliente, &ids[i]);
    }
    for (int i = 0; i < n_clientes; i++) {
        pthread_join(hilos[i], NULL);
    }

    double duracion_total = tiempo_transcurrido();
    double suma_rafagas = 0.0;

    for (int i = 0; i < n_clientes; i++) {
        suma_rafagas += clientes[i].rafaga;
    }

    double espera_promedio = suma_espera / n_clientes;
    double uso_pct = (suma_rafagas / (capacidad * duracion_total)) * 100.0;

    printf("\n=== Métricas finales ===\n");
    printf("Tiempo total de simulación: %.2f s\n", duracion_total);
    printf("Espera promedio: %.2f s\n", espera_promedio);
    printf("Uso del recurso: %.2f%%\n", uso_pct);

    fprintf(log_archivo, "\n=== Métricas finales ===\n");
    fprintf(log_archivo, "Tiempo total de simulación: %.2f s\n", duracion_total);
    fprintf(log_archivo, "Espera promedio: %.2f s\n", espera_promedio);
    fprintf(log_archivo, "Uso del recurso: %.2f%%\n", uso_pct);

    fclose(log_archivo);
    sem_destroy(&recurso);

    return 0;
}
```

### 7.4 Ejecución y logs

Se ejecutó el mismo escenario (8 clientes, semilla 3, FCFS) con dos
capacidades distintas para responder la pregunta 7.6.2.

#### Capacidad = 3 ventanillas (`./bin/simulador 8 3 --seed 3`)

```text
=== Simulación Banco/Caja ===
Clientes: 8 | Ventanillas: 3 | Modo: FCFS | Semilla: 3
  Cliente 1: llegada=2  atencion=2
  Cliente 2: llegada=0  atencion=1
  Cliente 3: llegada=0  atencion=2  (adulto mayor)
  Cliente 4: llegada=0  atencion=2  (adulto mayor)
  Cliente 5: llegada=1  atencion=2  (adulto mayor)
  Cliente 6: llegada=1  atencion=1
  Cliente 7: llegada=1  atencion=1  (adulto mayor)
  Cliente 8: llegada=6  atencion=2

[t=0] Cliente 4 LLEGA y solicita recurso
[t=0] Cliente 4 INGRESA (recurso ocupado: 1/3)
[t=0] Cliente 2 LLEGA y solicita recurso
[t=0] Cliente 2 INGRESA (recurso ocupado: 2/3)
[t=0] Cliente 3 LLEGA y solicita recurso
[t=0] Cliente 3 INGRESA (recurso ocupado: 3/3)
[t=1] Cliente 6 LLEGA y solicita recurso (en espera)
[t=1] Cliente 5 LLEGA y solicita recurso (en espera)
[t=1] Cliente 7 LLEGA y solicita recurso (en espera)
[t=1] Cliente 2 SALE (recurso ocupado: 2/3)
[t=1] Cliente 5 INGRESA (recurso ocupado: 3/3)
[t=2] Cliente 1 LLEGA y solicita recurso (en espera)
[t=2] Cliente 4 SALE (recurso ocupado: 2/3)
[t=2] Cliente 3 SALE (recurso ocupado: 1/3)
[t=2] Cliente 6 INGRESA (recurso ocupado: 2/3)
[t=2] Cliente 7 INGRESA (recurso ocupado: 3/3)
[t=3] Cliente 6 SALE (recurso ocupado: 2/3)
[t=3] Cliente 1 INGRESA (recurso ocupado: 3/3)
[t=3] Cliente 5 SALE (recurso ocupado: 2/3)
[t=3] Cliente 7 SALE (recurso ocupado: 1/3)
[t=5] Cliente 1 SALE (recurso ocupado: 0/3)
[t=6] Cliente 8 LLEGA y solicita recurso
[t=6] Cliente 8 INGRESA (recurso ocupado: 1/3)
[t=8] Cliente 8 SALE (recurso ocupado: 0/3)

=== Métricas finales ===
Tiempo total de simulación: 8.00 s
Espera promedio: 0.38 s
Uso del recurso: 54.16%
```

[CAPTURA: s4_simulador_cap3_exec.png]

#### Capacidad = 1 ventanilla (`./bin/simulador 8 1 --seed 3`)

```text
=== Simulación Banco/Caja ===
Clientes: 8 | Ventanillas: 1 | Modo: FCFS | Semilla: 3
  Cliente 1: llegada=2  atencion=2
  Cliente 2: llegada=0  atencion=1
  Cliente 3: llegada=0  atencion=2  (adulto mayor)
  Cliente 4: llegada=0  atencion=2  (adulto mayor)
  Cliente 5: llegada=1  atencion=2  (adulto mayor)
  Cliente 6: llegada=1  atencion=1
  Cliente 7: llegada=1  atencion=1  (adulto mayor)
  Cliente 8: llegada=6  atencion=2

[t=0] Cliente 4 LLEGA y solicita recurso
[t=0] Cliente 4 INGRESA (recurso ocupado: 1/1)
[t=0] Cliente 3 LLEGA y solicita recurso (en espera)
[t=0] Cliente 2 LLEGA y solicita recurso (en espera)
[t=1] Cliente 5 LLEGA y solicita recurso (en espera)
[t=1] Cliente 6 LLEGA y solicita recurso (en espera)
[t=1] Cliente 7 LLEGA y solicita recurso (en espera)
[t=2] Cliente 1 LLEGA y solicita recurso (en espera)
[t=2] Cliente 4 SALE (recurso ocupado: 0/1)
[t=2] Cliente 2 INGRESA (recurso ocupado: 1/1)
[t=3] Cliente 2 SALE (recurso ocupado: 0/1)
[t=3] Cliente 3 INGRESA (recurso ocupado: 1/1)
[t=5] Cliente 3 SALE (recurso ocupado: 0/1)
[t=5] Cliente 5 INGRESA (recurso ocupado: 1/1)
[t=6] Cliente 8 LLEGA y solicita recurso (en espera)
[t=7] Cliente 5 SALE (recurso ocupado: 0/1)
[t=7] Cliente 6 INGRESA (recurso ocupado: 1/1)
[t=8] Cliente 6 SALE (recurso ocupado: 0/1)
[t=8] Cliente 7 INGRESA (recurso ocupado: 1/1)
[t=9] Cliente 7 SALE (recurso ocupado: 0/1)
[t=9] Cliente 1 INGRESA (recurso ocupado: 1/1)
[t=11] Cliente 1 SALE (recurso ocupado: 0/1)
[t=11] Cliente 8 INGRESA (recurso ocupado: 1/1)
[t=13] Cliente 8 SALE (recurso ocupado: 0/1)

=== Métricas finales ===
Tiempo total de simulación: 13.00 s
Espera promedio: 4.25 s
Uso del recurso: 99.98%
```

[CAPTURA: s4_simulador_cap1_exec.png]

### 7.5 Tabla comparativa de métricas

Dataset: 8 clientes, semilla 3 (`./bin/simulador 8 <capacidad> --seed 3`).

| Capacidad (ventanillas) | Tiempo total simulación | Espera promedio | Uso del recurso |
|---:|---:|---:|---:|
| 3 | 8.00 s | 0.38 s | 54.16% |
| 1 | 13.00 s | 4.25 s | 99.98% |

Al pasar de 3 a 1 ventanillas, la espera promedio se **multiplica por ~11.2**
(de 0.38 s a 4.25 s) y el tiempo total de simulación crece de 8 s a 13 s,
mientras que el uso del recurso sube de 54.16% a casi 100% — el único cajero
queda saturado y la cola de espera crece notablemente.

### 7.6 Preguntas a responder

**1. ¿Qué algoritmo elegiste y por qué es el más adecuado para tu escenario?**

Se eligió **FCFS** (orden de llegada) con una extensión opcional de
prioridad (`--prioridad`) que da preferencia a los clientes "adulto mayor",
manteniendo FCFS dentro de cada grupo de prioridad. Es el algoritmo más
adecuado para una agencia bancaria porque: (a) es el criterio que los
clientes perciben como **justo** ("el que llegó primero, se atiende
primero"); (b) **no requiere conocer de antemano** cuánto durará la atención
de cada cliente, a diferencia de SJF, que sí necesita esa información; y
(c) RR no tiene sentido aquí porque la atención de un cliente en una
ventanilla no se puede "pausar y reanudar" a mitad de trámite. La extensión
de prioridad por adulto mayor refleja una política real de atención
preferencial sin caer en inanición total, ya que solo reordena dentro de la
cola de espera (sigue siendo FCFS entre pares).

**2. ¿Qué pasa con la espera promedio si reduces la capacidad (p. ej. de 3 a 1)? Pruébalo.**

Con el mismo dataset (8 clientes, semilla 3), la espera promedio pasa de
**0.38 s** (capacidad = 3) a **4.25 s** (capacidad = 1), es decir, se
multiplica por ~11.2x (ver tabla de la sección 7.5 y los logs de la sección
7.4). El uso del recurso sube de 54.16% a 99.98% y el tiempo total de
simulación crece de 8 s a 13 s. Esto es consistente con la teoría de colas:
al reducir el número de servidores manteniendo la misma carga de llegadas,
la utilización se acerca a 100% y la cola de espera (y por tanto el tiempo
de espera) crece de forma no lineal.

**3. ¿Dónde podría ocurrir una race condition sin el mutex/semáforo? Ejemplo concreto de tu código.**

Sin `mutex_estado`, la operación `ocupado++` / `ocupado--`
(`sesion4-integrador/simulador.c:154,164`) tiene exactamente el mismo
problema que `contador++` en `race_condition.c` (sección 6.2): no es
atómica (leer → incrementar → escribir), así que dos hilos podrían leer
`ocupado == 2` simultáneamente, ambos incrementarlo a 3 y escribir 3,
**perdiendo un incremento**. Peor aún, la condición `ocupado < capacidad`
dentro de `puede_pasar()` podría evaluarse con un valor de `ocupado`
desactualizado y dejar entrar a **más clientes que ventanillas
disponibles** — el invariante "ocupado ≤ capacidad" se rompería. Además,
sin `mutex_estado` protegiendo `registrar()` (`simulador.c:76-90`), dos
hilos podrían intercalar sus `printf`/`fprintf` a mitad de línea y el
`log_simulacion.txt` quedaría con líneas entremezcladas e ilegibles — el
gotcha documentado en `CLAUDE.md` para la Sesión 4.

**4. Compara con un SO real: ¿qué representa el recurso limitado? ¿Y los clientes?**

El semáforo contador `recurso` (inicializado en `capacidad`) representa
cualquier **recurso del sistema disponible en N instancias idénticas**: por
ejemplo, las `N` conexiones de un *connection pool* a una base de datos, los
`N` *slots* de un buffer de E/S, o los `N` núcleos de CPU que un
*scheduler* puede asignar a hilos listos para ejecutar. Los **clientes**
(hilos) representan los **procesos o hilos que compiten por ese recurso**
— en un SO real serían peticiones entrantes a un servidor, tareas en una
cola de trabajos, o hilos esperando una región crítica. La combinación
`sem_wait`/`sem_post` + `mutex_estado` + `cv_turno` es análoga al trabajo
del planificador y del subsistema de sincronización del kernel: decide
**quién** obtiene el recurso a continuación y **despierta** a los que
esperan cuando el recurso se libera, igual que `wait_queue` +
`wake_up` en Linux (Kerrisk, 2010).

**5. ¿Qué mejorarías con una sesión adicional?**

- Registrar y mostrar **métricas por cliente** (espera individual, TAT),
  no solo promedios, y un **Gantt de ocupación** de las ventanillas a lo
  largo del tiempo (qué cliente ocupó qué "ventanilla" lógica y cuándo).
- Implementar y comparar **otro criterio de selección** además de FCFS, por
  ejemplo SJF si se conociera de antemano la duración de atención, para
  contrastar el efecto en la espera promedio.
- Correr `valgrind --tool=helgrind ./bin/simulador` para verificar
  formalmente la ausencia de *data races* en `ocupado`, `cola[]` y
  `tam_cola`, además de la verificación manual de la sección 7.6.3.
- Permitir variar la `capacidad` **dinámicamente** durante la simulación
  (p. ej. abrir/cerrar ventanillas según la hora), lo que requeriría ajustar
  el semáforo en caliente (`sem_post`/`sem_wait` adicionales) protegido por
  `mutex_estado`.

---

## 8. Conclusiones generales

### 8.1 Sesión 1 — FCFS y SJF

Sobre el mismo dataset de 5 procesos, **SJF redujo la espera promedio de
4.60 a 4.00** frente a FCFS (sección 4.4), confirmando la propiedad teórica
de que SJF minimiza el tiempo de espera promedio cuando se conocen las
ráfagas de antemano. El costo es la **inanición** de procesos largos (P1,
con BT=5, es el último en ejecutarse en SJF a pesar de haber llegado
primero) y la necesidad de conocer `BT` por adelantado, algo poco realista
en un SO de propósito general. El módulo `common/` (struct `Proceso`,
`leer_procesos`, `calcular_metricas`, `gantt_imprimir`) permitió implementar
ambos algoritmos reutilizando exactamente la misma lógica de métricas y
Gantt, evitando duplicación y asegurando que las comparaciones de la sección
4.4 sean coherentes entre sí.

### 8.2 Sesión 2 — Round Robin y Prioridades

RR con quantum pequeño (q=2) **empeora** la espera y retorno promedio
(5.80 / 8.80) frente a FCFS/SJF en este dataset, pero a cambio **reduce el
tiempo de respuesta (RT)** de procesos cortos como P3 (BT=1), que en FCFS
debe esperar a que termine P1 (BT=5) pero en RR es atendido dentro de los
primeros `2·q` unidades de tiempo. El ejercicio 6.3 mostró que con
`q ≥ ráfaga máxima` (q=100), RR colapsa exactamente a FCFS — mismos
inicio/fin/espera/retorno por proceso, validando que RR es una
generalización de FCFS. Prioridades estáticas (sección 5.3) dieron la peor
espera promedio (5.40) de los cinco algoritmos porque P3 (prioridad 3, la
más baja) quedó relegado al final pese a tener la ráfaga más corta —
evidenciando el problema de **inanición por prioridad**, que se resuelve
con *aging* (sección 9.1).

### 8.3 Sesión 3 — Hilos y sincronización

La evidencia de la sección 6.2 es la conclusión más contundente del
laboratorio: **sin mutex, 3 corridas idénticas del mismo programa dan 3
resultados distintos y siempre incorrectos** (127843, 301450, 308451 en
lugar de 400000), mientras que con mutex las 3 corridas dan exactamente
400000. Esto demuestra de forma empírica que `contador++` no es atómico y
que la exclusión mutua no es opcional cuando varios hilos comparten estado
mutable. El productor-consumidor (sección 6.3) y su extensión 2P/2C
(ejercicio 7.1) mostraron que semáforos contadores (`vacios`/`llenos`) +
mutex permiten coordinar múltiples productores/consumidores sobre un buffer
circular sin perder ni duplicar elementos, siempre que se respete el orden
`sem_wait → lock → ... → unlock → sem_post` para evitar deadlock. El
ejercicio 7.2 cuantificó el costo de esa corrección: ~0.265 s adicionales
para 4 millones de incrementos, un precio bajo frente a la garantía de
corrección.

### 8.4 Sesión 4 — Proyecto integrador

El simulador Banco/Caja integró todos los mecanismos de las sesiones
anteriores (hilos, semáforo contador, mutex, variable de condición) en un
escenario único. El experimento de la sección 7.5/7.6.2 (capacidad 3 → 1)
mostró que reducir el recurso compartido a un tercio **multiplica la espera
promedio por ~11x** (0.38 s → 4.25 s), una relación claramente no lineal que
ilustra por qué el dimensionamiento de recursos compartidos (pools de
conexiones, núcleos, ventanillas) es crítico en sistemas reales.

### 8.5 Conclusión general

A lo largo de las 4 sesiones se construyó incrementalmente un mini-SO que
cubre los dos pilares centrales de la asignatura: **planificación de CPU**
(FCFS, SJF, RR, Prioridades, con sus métricas AT/BT/CT/TAT/WT/RT y diagramas
de Gantt) y **concurrencia** (hilos POSIX, condiciones de carrera, mutex,
semáforos y variables de condición). El hilo conductor entre ambos pilares
es la **gestión de recursos compartidos bajo restricciones**: en S1/S2 el
recurso es la CPU y el "scheduler" decide el orden; en S3/S4 el recurso es
memoria compartida o ventanillas, y mutex/semáforos cumplen el rol que el
planificador cumple a nivel de procesos. Haber reutilizado `common/` en
todas las sesiones de planificación, y haber verificado cada resultado con
ejecuciones reales y reproducibles (semillas fijas), reforzó la importancia
de la **reproducibilidad** y de **medir antes de afirmar** — especialmente
evidente en la sección 6.2, donde "ejecútalo varias veces" fue la única
forma de exponer el bug.

---

## 9. Respuestas de documentación

### 9.1 — Ejercicio 6.1: Aging para evitar inanición en Prioridades

**Problema.** En la planificación por prioridades estática (sección 5.3),
un proceso con prioridad numérica alta (p. ej. 3, la más baja en nuestra
convención 1=máxima, 5=mínima) puede quedar **indefinidamente postergado**
si llegan continuamente procesos con mejor prioridad. En el dataset de la
sección 5.1, P3 (prioridad 3) es el único caso real: aun teniendo la ráfaga
más corta (BT=1), se ejecuta **último** (Gantt: `0-5-8-10-14-15`), después
de P1, P2, P4 y P5 — todos con prioridad ≤ 2.

**Regla de aging propuesta.** Se define una constante
`UMBRAL_AGING = 5` (unidades de tiempo). Cada vez que el planificador avanza
el reloj y un proceso permanece en la cola de listos sin ser despachado, se
incrementa su contador `espera_acumulada`. Cuando
`espera_acumulada >= UMBRAL_AGING`, la prioridad numérica del proceso se
**decrementa en 1** (mejora, ya que 1 = máxima prioridad), con un límite
inferior de 1, y `espera_acumulada` se reinicia a 0. Cuando el proceso es
finalmente despachado, `espera_acumulada` se reinicia a 0 (su prioridad
mejorada se conserva para futuras reentradas a la cola, p. ej. si se usara
con RR+prioridades).

**Aplicación al dataset de la sección 5.1.** P3 (prioridad 3, AT=2) entra a
la cola de listos en t=2. Si para t=7 (5 unidades después) sigue sin ser
despachado, su prioridad pasaría de 3 a 2, empatando con P1 y P5; con el
criterio de desempate FCFS (AT, luego PID) ya documentado en este informe,
P3 (AT=2) pasaría delante de P5 (AT=4) la próxima vez que se elija un
proceso, evitando que quede relegado hasta el final. Esta regla garantiza
que **ningún proceso espere más de `(prioridad_inicial − 1) × UMBRAL_AGING`**
unidades de tiempo antes de alcanzar la prioridad máxima, acotando la
inanición.

### 9.2 — Ejercicio 6.4: Recomendación — sistemas interactivos vs. batch

| Característica | Sistema interactivo | Sistema batch |
|---|---|---|
| Métrica prioritaria | **RT** (tiempo de respuesta) | **TAT** y *throughput* |
| Algoritmo recomendado | **Round Robin** con quantum pequeño | **FCFS** o **SJF** |
| Justificación | El usuario espera realimentación rápida; RR garantiza que ningún proceso espere más de `(n-1)·q` antes de su primera ejecución | No hay usuario esperando interactivamente; minimizar el tiempo total de retorno y maximizar el uso de CPU es más valioso que la rapidez de respuesta individual |
| Riesgo si se usa el algoritmo contrario | FCFS/SJF en interactivo: un proceso largo bloquea la respuesta de todos los demás (efecto convoy, ver sección 9.x / ejercicio 5.3) | RR en batch: los cambios de contexto frecuentes (ejercicio 6.3 mostró 8 cambios con q=2 vs. 5 con q=4) añaden *overhead* sin beneficio, ya que no hay nadie esperando una respuesta rápida |

**Conclusión.** Para un sistema **interactivo** (terminal, GUI, servidor de
peticiones cortas) se recomienda **RR con quantum pequeño** (similar al
q=2 de la sección 5.2), priorizando RT sobre TAT. Para un sistema **batch**
(procesamiento por lotes, *jobs* nocturnos, compilaciones) se recomienda
**FCFS** (simplicidad, sin *overhead* de cambio de contexto) o **SJF** si se
puede estimar la duración de los trabajos, priorizando TAT y *throughput*
sobre RT — como se observó en la sección 4.4, SJF reduce la espera promedio
de 4.60 a 4.00 frente a FCFS en nuestro dataset.

### 9.3 — Ejercicio 7.3: Mutex vs. semáforo

**Mutex (exclusión mutua binaria).** Un `pthread_mutex_t` tiene exactamente
dos estados (bloqueado/libre) y un **propietario**: el hilo que llama a
`pthread_mutex_lock` debe ser el mismo que llama a
`pthread_mutex_unlock`. Su único propósito es proteger una **sección
crítica** para que, como máximo, un hilo a la vez modifique un dato
compartido. En `race_condition.c` (sección 6.2), `mutex_contador` protege
`contador++`; en `productor_consumidor.c` (sección 6.3), `mutex_buffer`
protege los índices `pos_in`/`pos_out` y el arreglo `buffer[]`; en
`simulador.c` (sección 7), `mutex_estado` protege `ocupado`, `cola[]`,
`tam_cola` y la función `registrar()` (consola + log).

**Semáforo (contador, sin propietario).** Un `sem_t` mantiene un **contador
entero no negativo**. `sem_wait` lo decrementa (bloqueando si llega a
negativo) y `sem_post` lo incrementa; **cualquier** hilo puede hacer
`sem_post`, no necesariamente el que hizo `sem_wait` — no hay noción de
"propietario". Esto lo hace ideal para **contar recursos disponibles**: en
`productor_consumidor.c`, `vacios` (inicial = `TAM_BUFFER`) cuenta huecos
libres y `llenos` (inicial = 0) cuenta elementos listos para consumir; en
`simulador.c`, `sem_t recurso` (inicial = `capacidad`) cuenta ventanillas
libres — un semáforo con valor inicial > 1 (semáforo *contador*) generaliza
la exclusión mutua a "como máximo N hilos a la vez", algo que un mutex no
puede expresar directamente.

**Resumen de uso en este proyecto.**

| Mecanismo | Valor inicial | Uso en el proyecto |
|---|---|---|
| `pthread_mutex_t` | "libre" (1 implícito) | Proteger **datos compartidos mutables** (contador, índices de buffer, `ocupado`, log) — sección 6.2 y 7 |
| `sem_t` (binario, init=0) | 0 | Señalizar **eventos** entre hilos: `llenos` (hay algo para consumir) — sección 6.3 |
| `sem_t` (contador, init=N) | N | Limitar el acceso concurrente a **N unidades de un recurso**: `vacios` (huecos del buffer), `recurso` (ventanillas) — secciones 6.3 y 7 |

En síntesis: **mutex = exclusión mutua de un dato** (1 hilo a la vez, con
propietario); **semáforo = contador de disponibilidad de un recurso** (N
hilos a la vez, sin propietario, y también usable para señalización entre
hilos).

---

## 10. Referencias (APA 7)

Kerrisk, M. (2010). *The Linux programming interface: A Linux and UNIX system programming handbook*. No Starch Press.

Silberschatz, A., Galvin, P. B., & Gagne, G. (2018). *Operating system concepts* (10th ed.). John Wiley & Sons.

Stallings, W. (2018). *Operating systems: Internals and design principles* (9th ed.). Pearson.

Tanenbaum, A. S., & Bos, H. (2015). *Modern operating systems* (4th ed.). Pearson.
