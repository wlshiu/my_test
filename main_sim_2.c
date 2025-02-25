#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define fatal(args...)     \
    do {                   \
        printf("Fatal: "); \
        printf(args);      \
        exit(-1);          \
    } while (0)

enum
{
    /* I-type integer computation or integer register-immediate */
    INTEGER_COMP_RI = 0x13,
    LUI = 0x37,
    AUIPC = 0x17,
    ECALL = 0x73,
};

enum
{
    ADDI = 0x0,
    /* RV32M opcodes */
    MUL = 0x0,
    MULH = 0x1,
    DIV = 0x4,
    DIVU = 0x5,
    REM = 0x6,
    REMU = 0x7,
};

/* FUNCT3 for INTEGER_COMP_RI */
enum
{
    OPCODE_MASK = 0x7F,
    REG_ADDR_MASK = 0x1F
};

enum
{
    RD_SHIFT = 7,
    RS1_SHIFT = 15,
    RS2_SHIFT = 20,
    FUNCT3_SHIFT = 12,
    FUNCT7_SHIFT = 25
};
enum
{
    FUNCT3_MASK = 0x7,
    FUNCT7_MASK = 0x7F
};

enum
{
    RAM_SIZE = 1204 * 1024 * 2,
    RAM_BASE = 0x0
};

typedef struct
{
    uint8_t *mem;
} ram_t;

typedef struct
{
    uint32_t regs[32];
    size_t pc;
    ram_t *ram;
} cpu_t;

typedef struct
{
    uint8_t opcode;
    uint8_t rd, rs1, rs2, funct3, funct7;
} insn_t;

ram_t *ram_new(uint8_t *code, size_t len)
{
    ram_t *ram = malloc(sizeof(ram_t));
    ram->mem = malloc(RAM_SIZE);
    memcpy(ram->mem, code, len);
    return ram;
}

void ram_free(ram_t *mem)
{
    free(mem->mem);
    free(mem);
}

static inline void check_addr(int32_t addr)
{
    size_t index = addr - RAM_BASE;
    if (index >= RAM_SIZE)
        fatal("out of memory.\n");
}

static uint32_t ram_load8(ram_t *mem, uint32_t addr)
{
    size_t index = addr - RAM_BASE;
    return (uint32_t) mem->mem[index];
}

static uint32_t ram_load16(ram_t *mem, uint32_t addr)
{
    size_t index = addr - RAM_BASE;
    return ((uint32_t) mem->mem[index] | ((uint32_t) mem->mem[index + 1] << 8));
}

static uint32_t ram_load32(ram_t *mem, uint32_t addr)
{
    size_t index = addr - RAM_BASE;
    return ((uint32_t) mem->mem[index] | ((uint32_t) mem->mem[index + 1] << 8) |
            ((uint32_t) mem->mem[index + 2] << 16) |
            ((uint32_t) mem->mem[index + 3] << 24));
}

uint32_t ram_load(ram_t *mem, uint32_t addr, uint8_t size)
{
    check_addr(addr);
    uint32_t r = 0;
    switch (size)
    {
    case 8:
        r = ram_load8(mem, addr);
        break;
    case 16:
        r = ram_load16(mem, addr);
        break;
    case 32:
        r = ram_load32(mem, addr);
        break;
    default:
        fatal("RAM: invalid load size(%d)\n", size);
    }

    return r;
}

cpu_t *cpu_new(uint8_t *code, size_t len)
{
    cpu_t *cpu = malloc(sizeof(cpu_t));
    memset(cpu->regs, 0, sizeof(cpu->regs));
    cpu->regs[0] = 0; // x0 is always zero
    /* Stack pointer */
    cpu->regs[2] = RAM_BASE + RAM_SIZE;
    cpu->pc = RAM_BASE;
    cpu->ram = ram_new(code, len);
    return cpu;
}

void cpu_free(cpu_t *cpu)
{
    ram_free(cpu->ram);
    free(cpu);
}

uint32_t cpu_load(cpu_t *cpu, uint32_t addr, uint8_t size)
{
    return ram_load(cpu->ram, addr, size);
}

uint32_t cpu_fetch(cpu_t *cpu)
{
    return cpu_load(cpu, cpu->pc, 32);
}

void cpu_decode(uint32_t raw, insn_t *inst)
{
    uint8_t opcode = raw & OPCODE_MASK;
    uint8_t rd = (raw >> RD_SHIFT) & REG_ADDR_MASK;
    uint8_t rs1 = (raw >> RS1_SHIFT) & REG_ADDR_MASK;
    uint8_t rs2 = (raw >> RS2_SHIFT) & REG_ADDR_MASK;
    uint8_t funct3 = (raw >> FUNCT3_SHIFT) & FUNCT3_MASK;
    uint8_t funct7 = (raw >> FUNCT7_SHIFT) & FUNCT7_MASK;

    inst->opcode = opcode;
    inst->rd = rd, inst->rs1 = rs1, inst->rs2 = rs2;
    inst->funct3 = funct3;
    inst->funct7 = funct7;
}

static inline int32_t i_imm(uint32_t raw)
{
    /* imm[11:0] = inst[31:20] */
    return ((int32_t) raw) >> 20;
}

static inline int32_t u_imm(uint32_t raw)
{
    /* imm[20:0] = inst[31:12] */
    return ((int32_t) raw & 0xFFFFF000);
}

enum { SYSCALL_WRITE = 64, SYSCALL_EXIT = 93 };

enum { STDOUT = 1, STDERR = 2 };

void ecall_handler(cpu_t *cpu)
{
    uint32_t syscall_nr = cpu->regs[17];

    switch (syscall_nr)
    {
    case SYSCALL_WRITE:
    {
        uint32_t fd = cpu->regs[10];
        uint32_t addr = cpu->regs[11];
        uint32_t count = cpu->regs[12];
        FILE *stream;

        switch (fd)
        {
        case STDOUT:
            stream = stdout;
            break;
        case STDERR:
            stream = stderr;
            break;
        default:
            fatal("invalid file-descriptor %d.\n", fd);
        }

        for (uint32_t i = 0; i < count; i++)
        {
            fprintf(stream, "%c", cpu_load(cpu, addr + i, 8));
        }
        break;
    }

    case SYSCALL_EXIT:
    {
        int32_t exit_code = cpu->regs[10];
        exit(exit_code);
        break;
    }

    default:
        fatal("unkown syscall number %d.\n", syscall_nr);
    }
}

static insn_t inst;

void cpu_execute(cpu_t *cpu, uint32_t raw)
{
    cpu_decode(raw, &inst);
    cpu->regs[0] = 0; // x0 is always zero

    switch (inst.opcode)
    {
        case INTEGER_COMP_RI:
        {
            int32_t imm = i_imm(raw);
            switch (inst.funct3)
            {
            case ADDI:
                cpu->regs[inst.rd] = cpu->regs[inst.rs1] + imm;
                break;
            default:
                fatal("Unknown FUNCT3 for INTEGER_COMP_RI");
            }
            break;
        }

        case LUI:
        {
            int32_t imm = u_imm(raw);
            cpu->regs[inst.rd] = imm;
            break;
        }

        case AUIPC:
        {
            int32_t imm = u_imm(raw);
            cpu->regs[inst.rd] = (cpu->pc + (-4)) + imm;
            break;
        }

        // RV32M 指令集 (乘法與除法)
        case 0x33:    // RV32M 是 0x33 opcode
        {
            switch (inst.funct3)
            {
            case 0x0:  // MUL
                if (inst.funct7 == 0x01)
                {
                    cpu->regs[inst.rd] = (int32_t)cpu->regs[inst.rs1] * (int32_t)cpu->regs[inst.rs2];
                }
                break;
            case 0x1:  // MULH
                if (inst.funct7 == 0x01)
                {
                    int64_t result = (int64_t)(int32_t)cpu->regs[inst.rs1] * (int32_t)cpu->regs[inst.rs2];
                    cpu->regs[inst.rd] = (result >> 32) & 0xFFFFFFFF;
                }
                break;
            case 0x4:  // DIV
                if (inst.funct7 == 0x01)
                {
                    cpu->regs[inst.rd] = cpu->regs[inst.rs1] / cpu->regs[inst.rs2];
                }
                break;
            case 0x5:  // DIVU
                if (inst.funct7 == 0x01)
                {
                    cpu->regs[inst.rd] = (unsigned int)cpu->regs[inst.rs1] / (unsigned int)cpu->regs[inst.rs2];
                }
                break;
            case 0x6:  // REM
                if (inst.funct7 == 0x01)
                {
                    cpu->regs[inst.rd] = cpu->regs[inst.rs1] % cpu->regs[inst.rs2];
                }
                break;
            case 0x7:  // REMU
                if (inst.funct7 == 0x01)
                {
                    cpu->regs[inst.rd] = (unsigned int)cpu->regs[inst.rs1] % (unsigned int)cpu->regs[inst.rs2];
                }
                break;
            default:
                fatal("Unknown funct3 for RV32M instruction\n");
            }
            break;
        }

        // RV32C 指令集
        case 0x1:    // set RV32C指令的opcode是0x1
        {
            switch (inst.funct3)
            {
            case 0x0:  // C.ADDI
                cpu->regs[inst.rd] = cpu->regs[inst.rs1] + i_imm(raw);
                break;
            // 其他 RV32C 指令
            default:
                fatal("Unknown FUNCT3 for RV32C instruction\n");
            }
            break;
        }

        case ECALL:
            if (raw == 0x100073)
                break;  /* EBREAK */
            ecall_handler(cpu);
            break;

        default:
            fatal("Illegal Instruction 0x%x\n", inst.opcode);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s [filename]\n", argv[0]);
        exit(-1);
    }

    const char *filename = argv[1];
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        printf("Failed to open %s\n", filename);
        exit(-1);
    }
    fseek(file, 0L, SEEK_END);
    size_t code_size = ftell(file);
    rewind(file);

    uint8_t *code = malloc(code_size);
    fread(code, sizeof(uint8_t), code_size, file);
    fclose(file);

    cpu_t *cpu = cpu_new(code, code_size);

    while (1)
    {
        uint32_t raw = cpu_fetch(cpu);
        cpu->pc += 4;
        if (cpu->pc > code_size)
            break;
        cpu_execute(cpu, raw);

        if (!cpu->pc)
            break;
    }

    cpu_free(cpu);
    return 0;
}
