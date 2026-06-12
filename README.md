# Mini Sistema Operativo — Lab N°09

Proyecto integrador de **Sistemas Operativos** (UCSM, Ing. Gustavo Reinoso): simulador en
C/Linux que combina **planificación de CPU** (FCFS, SJF, Round Robin, Prioridades) con
**concurrencia** (pthreads, mutex, semáforos POSIX).

## Estructura

```
.
├── common/                 # struct Proceso, lectura de datos, métricas y Gantt
├── sesion1-scheduling/     # FCFS y SJF
├── sesion2-rr-prioridades/ # Round Robin, Prioridades y comparativa
├── sesion3-hilos/          # hilos básicos, race condition, productor-consumidor
├── sesion4-integrador/      # simulador integrador
├── ejercicios/             # ejercicios propuestos (s1_*, s2_*, s3_*)
└── informe/                # informe.md y capturas/
```

## Compilación

```bash
make            # compila todas las sesiones en ./bin
make s1         # Sesión 1
make s2         # Sesión 2
make s3         # Sesión 3
make s4         # Sesión 4
make ejercicios # ejercicios propuestos
make clean      # borra ./bin y *.o
```

Compilación con `-Wall -Wextra -std=c11` y cero warnings. Las sesiones 3 y 4 (y los
ejercicios que usan hilos) enlazan con `-pthread`. Los binarios se generan en `./bin/`.

## Verificación de concurrencia

```bash
valgrind --tool=helgrind ./bin/race_condition
```

## Documentación

La documentación canónica de cada sesión vive en Notion (ver `CLAUDE.md` para los enlaces).
