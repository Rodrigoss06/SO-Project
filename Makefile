# Makefile raíz — Mini Sistema Operativo (Lab N°09)
# Compila todas las sesiones y ejercicios hacia ./bin
#
# Targets:
#   make / make all   -> compila todo
#   make s1           -> Sesión 1 (FCFS, SJF)
#   make s2           -> Sesión 2 (Round Robin, Prioridades, comparativa)
#   make s3           -> Sesión 3 (hilos, race condition, productor-consumidor)
#   make s4           -> Sesión 4 (simulador integrador)
#   make ejercicios   -> ejercicios propuestos
#   make clean        -> borra ./bin y *.o

CC      := gcc
CFLAGS  := -Wall -Wextra -std=c11
PTHREAD := -pthread
BINDIR  := bin

COMMON_DIR := common
COMMON_SRC := $(wildcard $(COMMON_DIR)/*.c)

S1_DIR := sesion1-scheduling
S2_DIR := sesion2-rr-prioridades
S3_DIR := sesion3-hilos
S4_DIR := sesion4-integrador
EJ_DIR := ejercicios

S1_SRC := $(wildcard $(S1_DIR)/*.c)
S2_SRC := $(wildcard $(S2_DIR)/*.c)
S3_SRC := $(wildcard $(S3_DIR)/*.c)
S4_SRC := $(wildcard $(S4_DIR)/*.c)
EJ_SRC := $(wildcard $(EJ_DIR)/*.c)

S1_BIN := $(patsubst $(S1_DIR)/%.c,$(BINDIR)/%,$(S1_SRC))
S2_BIN := $(patsubst $(S2_DIR)/%.c,$(BINDIR)/%,$(S2_SRC))
S3_BIN := $(patsubst $(S3_DIR)/%.c,$(BINDIR)/%,$(S3_SRC))
S4_BIN := $(patsubst $(S4_DIR)/%.c,$(BINDIR)/%,$(S4_SRC))
EJ_BIN := $(patsubst $(EJ_DIR)/%.c,$(BINDIR)/%,$(EJ_SRC))

.PHONY: all s1 s2 s3 s4 ejercicios clean

all: s1 s2 s3 s4 ejercicios

s1: $(S1_BIN)
s2: $(S2_BIN)
s3: $(S3_BIN)
s4: $(S4_BIN)
ejercicios: $(EJ_BIN)

$(BINDIR):
	mkdir -p $(BINDIR)

# Sesión 1: sin pthread
$(BINDIR)/%: $(S1_DIR)/%.c $(COMMON_SRC) | $(BINDIR)
	$(CC) $(CFLAGS) -I$(COMMON_DIR) $^ -o $@

# Sesión 2: sin pthread
$(BINDIR)/%: $(S2_DIR)/%.c $(COMMON_SRC) | $(BINDIR)
	$(CC) $(CFLAGS) -I$(COMMON_DIR) $^ -o $@

# Sesión 3: requiere pthread
$(BINDIR)/%: $(S3_DIR)/%.c $(COMMON_SRC) | $(BINDIR)
	$(CC) $(CFLAGS) -I$(COMMON_DIR) $^ -o $@ $(PTHREAD)

# Sesión 4: requiere pthread
$(BINDIR)/%: $(S4_DIR)/%.c $(COMMON_SRC) | $(BINDIR)
	$(CC) $(CFLAGS) -I$(COMMON_DIR) $^ -o $@ $(PTHREAD)

# Ejercicios: se enlazan con pthread por si los de S3 lo requieren
$(BINDIR)/%: $(EJ_DIR)/%.c $(COMMON_SRC) | $(BINDIR)
	$(CC) $(CFLAGS) -I$(COMMON_DIR) $^ -o $@ $(PTHREAD)

clean:
	rm -rf $(BINDIR)
	find . -name '*.o' -delete
