PREFIX = riscv64-unknown-elf
CC = $(PREFIX)-gcc
CXX = $(PREFIX)-g++
LD = ../script/$(PREFIX)-ld-wrapper

OBJDUMP = $(PREFIX)-objdump
READELF = $(PREFIX)-readelf

SRC = \
  func1.c \
  func2.c \
  overlay_manager.c \
  overlay_stub.c \

SRC_CXX = \
  main.cpp \
  foo.cpp

OBJ = $(patsubst %.c,%.o,$(SRC))
OBJ_CXX = $(patsubst %.cpp,%.o,$(SRC_CXX))
EXE = overlay.exe
DUMP = $(EXE).objdump
SYM = $(EXE).sym
LDSCRIPT = overlay.ld

CFLAGS = -Og -g -msmall-data-limit=0
LDFLAGS = -T $(LDSCRIPT) -Wl,-Map=map.txt -mno-relax --verbose

all: $(EXE) $(DUMP)

$(EXE): $(OBJ) $(OBJ_CXX) $(LDSCRIPT)
	$(LD) $(LDFLAGS) $(OBJ) $(OBJ_CXX) -o $(EXE)

$(DUMP): $(EXE)
	$(OBJDUMP) --disassemble-all --section-headers --source $(EXE) > $@

$(SYM): $(EXE)
	$(READELF) -Ss $(EXE) > $(@)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@


.PHONY: clean
clean:
	rm -f $(OBJ) $(OBJ_CXX) $(EXE)
