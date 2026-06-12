# CLAUDE.md — Mini Sistema Operativo (Lab N°09)

Proyecto integrador de **Sistemas Operativos** (UCSM, Ing. Gustavo Reinoso): simulador
en **C/Linux** que combina **planificación de CPU** (FCFS, SJF, Round Robin, Prioridades)
con **concurrencia** (pthreads, mutex, semáforos POSIX).

## Fuente de verdad (Notion vía MCP)

La documentación canónica vive en Notion. **Antes de implementar una sesión, lee la
sub-página correspondiente con la herramienta de Notion (`fetch`) y respeta sus
"Definición de Hecho" y la salida esperada.** No inventes formatos: cópialos del Notion.

- Hub: https://app.notion.com/p/37dc3fa6d4ce81c0b5a0ef929c8d60fe
- Sesión 1 (FCFS/SJF): https://app.notion.com/p/37dc3fa6d4ce8117a6a0d1578862e7ff
- Sesión 2 (RR/Prioridades): https://app.notion.com/p/37dc3fa6d4ce8103ad0dfd889e36e661
- Sesión 3 (Hilos/Sincronización): https://app.notion.com/p/37dc3fa6d4ce81fda05cff8bf1bef599
- Sesión 4 (Integrador): https://app.notion.com/p/37dc3fa6d4ce8190a7daf3b2be75379f
- Ejercicios (tracker): https://app.notion.com/p/37dc3fa6d4ce81f69616fd8dcc209a39
- Informe y entregables: https://app.notion.com/p/37dc3fa6d4ce81b689e9d5da89ea371f

Al terminar una actividad o ejercicio, **marca el checkbox correspondiente en el tracker
de Notion** (`notion-update-page`) para mantener el progreso sincronizado.

## Entorno

- SO: Arch Linux. Compilador: `gcc` (C11).
- Concurrencia: POSIX threads → **siempre `-pthread`** en los programas de la Sesión 3 y 4.
- Verificación: `gcc --version`, `ps -eLf`, `top -H`, `valgrind --tool=helgrind`.

## Comandos de build / test

```bash
make            # compila todas las sesiones en ./bin
make s1         # compila solo Sesión 1 (fcfs, sjf)
make s2         # compila solo Sesión 2 (round_robin, prioridades, comparativa)
make s3         # compila solo Sesión 3 (hilos_basico, race_condition, productor_consumidor)
make s4         # compila el simulador integrador
make ejercicios # compila todo lo de ejercicios/
make clean      # borra ./bin y *.o

# Verificación de race conditions:
valgrind --tool=helgrind ./bin/race_condition
```

Reglas de compilación: **`-Wall -Wextra -std=c11`** y **cero warnings**. Los binarios van
a `./bin/`. Nunca commitear binarios ni `log_simulacion.txt`.

## Estructura del repo

```
mini-so-lab09/
├── CLAUDE.md
├── Makefile
├── common/                 # código compartido (NO duplicar lógica de métricas/Gantt)
│   ├── proceso.h / proceso.c
│   └── gantt.h / gantt.c
├── sesion1-scheduling/     fcfs.c  sjf.c
├── sesion2-rr-prioridades/ round_robin.c  prioridades.c  comparativa.c
├── sesion3-hilos/          hilos_basico.c  race_condition.c  productor_consumidor.c
├── sesion4-integrador/     simulador.c
├── ejercicios/             s1_*.c  s2_*.c  s3_*.c
└── informe/                informe.md  capturas/
```

## Convenciones de código (obligatorias)

- **Idioma:** código, identificadores y comentarios en **español** (consistente con la
  salida esperada del enunciado).
- **Métricas estándar:** `AT` llegada, `BT` ráfaga, `CT` finalización,
  `TAT = CT - AT` (retorno), `WT = TAT - BT` (espera), `RT = inicio - AT` (respuesta).
- **No duplicar:** la `struct Proceso`, la lectura de datos, el cálculo de métricas y el
  Gantt viven en `common/`. Los algoritmos solo implementan la lógica de selección.
- **Tablas:** alineadas con `printf("%4d")`; promedios con `%.2f`.
- **Gantt en texto:** barras `| Px |` + línea de tiempos debajo, agrupando ráfagas
  consecutivas del mismo PID. Formato idéntico al del Notion.
- **Cabecera de cada `.c`:** comentario con propósito, sesión/actividad y cómo compilar.
- **Aleatoriedad:** `srand()` con semilla; aceptar un flag opcional `--seed N` para
  reproducibilidad al depurar.
- **Robustez de concurrencia:** sin variables globales compartidas sin proteger; cada hilo
  recibe su dato por puntero a memoria propia (nunca `&i` de un bucle).

## Flujo de trabajo esperado de Claude Code

1. Leer la sub-página de Notion de la sesión objetivo.
2. Implementar primero `common/` si la sesión lo necesita y aún no existe.
3. Implementar la actividad, compilar con `make sN`, ejecutar y verificar contra la salida
   esperada del Notion.
4. Para concurrencia, correr varias veces (y helgrind donde aplique) y reportar resultados.
5. Marcar el checkbox en el tracker de Notion y, si procede, dejar nota en `informe/informe.md`.
6. Hacer commits atómicos por actividad (`feat(s1): FCFS con métricas y Gantt`).

## Gotchas conocidos

- **RR:** definir convención de reinserción cuando un proceso llega en el mismo `t` en que
  otro agota su quantum (primero el recién llegado, luego el desalojado). Contar cambios de
  contexto correctamente.
- **SJF/Prioridades:** manejar el caso "no hay procesos disponibles" avanzando `t` hasta la
  próxima llegada (CPU idle), no romper el bucle.
- **Productor-Consumidor:** orden `sem_wait(vacios) → lock → ... → unlock → sem_post(llenos)`
  (y simétrico en el consumidor). No invertir mutex y semáforos o habrá deadlock.
- **Simulador S4:** proteger con mutex tanto el contador `ocupado` como la escritura del log,
  o las líneas saldrán entremezcladas y el conteo será erróneo.
