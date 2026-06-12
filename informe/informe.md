# Informe — Mini Sistema Operativo (Lab N°09)

## Ejercicios propuestos — respuestas de documentación

### 6.1 — Aging (envejecimiento de prioridades)

**Problema que resuelve:** en planificación por prioridades (apropiativa o
no), un proceso de baja prioridad puede quedar **inanición (starvation)**
si llegan continuamente procesos de mayor prioridad: nunca consigue la CPU
porque siempre hay alguien "más urgente" delante de él en la cola.

**Idea de aging:** la prioridad de un proceso ya no es fija; aumenta
(mejora) cuanto más tiempo lleva esperando en la cola de listos. Así, todo
proceso termina alcanzando la prioridad más alta y se garantiza que se
ejecute en algún momento.

**Regla concreta de incremento (propuesta para `prioridades.c`):**

- Cada proceso tiene una prioridad base `prioridad` (1 = mayor, 5 = menor)
  y un contador `espera_acumulada`.
- En cada unidad de tiempo `t` en que el planificador NO despacha a un
  proceso que está en la cola de listos, se incrementa su
  `espera_acumulada`.
- Cada vez que `espera_acumulada` alcanza un umbral `UMBRAL_AGING`
  (por ejemplo, `UMBRAL_AGING = 5` unidades de tiempo), se decrementa en 1
  su prioridad efectiva (`prioridad_efectiva = max(1, prioridad_efectiva - 1)`)
  y se reinicia `espera_acumulada = 0`.
- Cuando el proceso es finalmente despachado, `prioridad_efectiva` vuelve a
  su valor `prioridad` original y `espera_acumulada` se reinicia a 0 (el
  envejecimiento solo aplica mientras el proceso está esperando).

**Efecto esperado:** un proceso de prioridad 5 que espera 20 unidades de
tiempo sin ejecutarse sube a prioridad 1 (`5 -> 4 -> 3 -> 2 -> 1`, un nivel
cada 5 unidades) y pasa a competir en igualdad de condiciones con los
procesos más urgentes, garantizando que eventualmente se le asigne la CPU.

---

### 6.4 — Recomendación: planificación interactiva vs. batch

**Sistema interactivo (terminal, escritorio, servidores con usuarios
esperando respuesta):** se recomienda **Round Robin con quantum pequeño**
(o una variante de prioridades con aging, como en Sesión 2/6.1). La métrica
crítica aquí es el **tiempo de respuesta (RT)**: el usuario necesita ver
que el sistema "reacciona" rápido, aunque su tarea completa tarde más en
terminar (mayor TAT). RR con quantum pequeño reparte la CPU en porciones
cortas entre todos los procesos listos, manteniendo el RT bajo y acotado
incluso con muchos procesos.

**Sistema batch (procesamiento por lotes, sin usuarios interactivos
esperando, p. ej. reportes nocturnos, compilaciones masivas):** se
recomienda **FCFS o SJF** (o Prioridades no apropiativas). Aquí lo que
importa es el **throughput** y el **tiempo de retorno promedio (TAT)**, no
la respuesta inmediata. SJF minimiza el TAT promedio (Sesión 1, ejercicio
5.3: el efecto convoy demuestra cómo procesos cortos detrás de uno largo
disparan la espera promedio en FCFS, y SJF lo evita). Como no hay un
usuario esperando feedback continuo, el costo de cambio de contexto extra
de RR no se justifica: cada cambio de contexto es overhead puro que no
mejora la experiencia de nadie.

**Resumen:**

| Entorno      | Algoritmo recomendado          | Métrica que se prioriza |
|--------------|---------------------------------|--------------------------|
| Interactivo  | RR (quantum pequeño) / Prioridades con aging | RT (tiempo de respuesta) |
| Batch        | SJF / FCFS                       | TAT y throughput |

---

### 7.3 — Mutex vs. semáforo (con ejemplo propio)

**Mutex (`pthread_mutex_t`):** mecanismo de **exclusión mutua binaria**.
Solo puede estar en dos estados (bloqueado/libre) y tiene la noción de
**propietario**: el hilo que hace `lock()` es, conceptualmente, el único
que debería hacer `unlock()`. Sirve para proteger una **sección crítica**
(un recurso/variable que no debe ser modificado por más de un hilo a la
vez).

**Semáforo (`sem_t`):** es un **contador** con dos operaciones atómicas,
`sem_wait` (decrementa, bloquea si el contador llega a 0) y `sem_post`
(incrementa, despierta a quien esté esperando). No tiene noción de
propietario: cualquier hilo puede hacer `sem_post`, incluso uno distinto
al que hizo `sem_wait`. Sirve tanto para exclusión mutua (semáforo binario,
inicializado en 1) como, sobre todo, para **señalización entre hilos** y
para **contar recursos disponibles** (un semáforo inicializado en N permite
que hasta N hilos entren a la vez).

**Diferencia clave:** un mutex protege un recurso compartido para que solo
un hilo lo toque "ahora"; un semáforo además puede coordinar *cuántos*
hilos pueden hacerlo simultáneamente y permite que la señal de "liberar" la
dé un hilo distinto al que esperó.

**Ejemplos propios de este proyecto:**

- **Mutex** — `sesion4-integrador/simulador.c`: `mutex_estado` protege el
  contador `ocupado` y la escritura en `log_simulacion.txt`. Solo un hilo
  a la vez puede leer/modificar `ocupado` o escribir una línea del log;
  no hay "conteo" involucrado, solo exclusión mutua de una sección crítica.

- **Semáforo** — el mismo `simulador.c` usa `sem_t recurso` inicializado
  en `capacidad` (número de ventanillas, M). Hasta M clientes pueden
  "pasar" (hacer `sem_wait`) simultáneamente sin bloquearse; el cliente
  M+1 se bloquea hasta que algún otro hilo (no necesariamente el mismo que
  esperó) llama `sem_post` al salir. Esto es justamente lo que un mutex no
  podría expresar directamente, porque un mutex solo admite "1 a la vez",
  no "hasta M a la vez".
