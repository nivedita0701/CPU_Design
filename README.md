# CMPE 220 – Software CPU Design Project (Part 1)

**Course:** CMPE 220 – System Software  
**Instructor:** *Prof. Ishie Eshwar* 
**Project:** Part 1 – Software CPU Design  


## Overview

This repository contains the **Part 1** implementation of a 16-bit Software CPU written in C++.  
It includes:

- A complete **CPU emulator** (fetch–decode–execute loop)
- An **assembler** that translates assembly into machine code
- A **memory subsystem** with a simple memory map and memory-mapped I/O
- Example programs:
  - **Hello, World**
  - **Fibonacci Sequence**
  - a simple “timer” / loop demo to illustrate Fetch/Compute/Store


## CPU Architecture (Part 1)

### Register File

- 6 general-purpose registers: **R0–R5**
- **FP (R6)** – frame pointer (not heavily used in Part 1 but available)
- **SP (R7)** – stack pointer (used for basic stack operations / future extensions)
- **PC** – program counter (16-bit)
- **IR** – instruction register (16-bit)
- **Flags** – Zero (Z), Negative (N)

All registers are 16-bit.


## Instruction Set (Part 1)

### Instruction Word Layout (16 bits)

|15 ... 12|11 ... 8|7 ... 4|3 ... 0|
| OPCODE | RD | RS1 | RS2/IMM4 |


### Supported Instructions in Part 1

| Mnemonic           | Opcode | Type | Description                                 |
|--------------------|:------:|:-----|---------------------------------------------|
| `LOADI rD, #imm8`  |  0x1   | I8   | Load 8-bit immediate into `rD`              |
| `LOAD rD, [rA]`    |  0x2   | R/M  | Load word from memory\[rA] into `rD`        |
| `STORE rS, [rA]`   |  0x3   | R/M  | Store word from `rS` into memory\[rA]       |
| `ADD rD, rS1, rS2` |  0x4   | R    | `rD = rS1 + rS2`                            |
| `ADDI rD, rS, #imm4`| 0x5   | I4   | `rD = rS + imm4` (signed 4-bit)             |
| `JUMP label`       |  0x9   | J    | Unconditional jump                          |
| `CMP rL, rR`       |  0xA   | R    | Compare `rL` and `rR`, set flags Z/N        |
| `JUMPEQ label`     |  0xB   | J    | Jump if Zero flag is set                    |
| `HALT`             |  0xF   | —    | Stop execution                              |



## Memory Map (Part 1)

The CPU uses a 16-bit address space (64 KiB) divided into code, data, stack, and MMIO regions.

```
Address (hex, 64 KiB total)

0xFFFF  +------------------------------------+
        |            MMIO Region             |
0xFF00  +------------------------------------+  ← IO_START (also STACK_START)
        |               STACK                |  grows downward (0xFF00 → …)
        |                 …                  |
0x1000  +------------------------------------+  ← DATA_START
        |                DATA                |  .DATA segment – strings, constants
        |                 …                  |
0x0000  +------------------------------------+  ← CODE_START
        |                CODE                |  Program instructions (~4 KiB)
        +------------------------------------+
```


- `CODE_START = 0x0000`
- `DATA_START = 0x1000`
- `STACK_START = 0xFF00`
- `IO_START = 0xFF00` (memory-mapped output register)

Writing a byte/word to the MMIO address (0xFF00) prints a character (used by Hello World).


## Emulator Components

All implementation is in the `src/` directory:

- `cpu_defs.h`  
  - Architecture constants (word size, memory size, addresses)  
  - Register indices and opcodes

- `Cpu.h` / `Cpu.cpp`  
  - Register file, PC, IR, flags  
  - `fetch()`, `decode_execute()`, `run()`  
  - ALU operations (ADD, ADDI, CMP, flags)

- `Memory.h` / `Memory.cpp`  
  - 64 KiB RAM array  
  - `read_word`, `write_word`, `load_byte`  
  - `dump_memory` for debugging  
  - Memory-mapped output at `IO_START`

- `Assembler.h` / `Assembler.cpp`  
  - Two-pass assembler (`pass1` for labels, `pass2` for encoding)  
  - Supports labels, `.DATA`, immediates (`#5`, `0x1000`, `#-1`)  
  - Loads machine code + data into `Memory`

- `main.cpp`  
  - Command line interface:  
    - `run <file.asm>` – assemble and run  
    - (Optionally) `step <file.asm>` – step through instructions  
  - Prints final register state and a memory dump


## Build & Run (Part 1)

### Build

From the project root:

```bash
g++ -std=c++11 -I./src src/*.cpp -o emulator
```


Hello World Program

```bash
./emulator run asm/hello_world.asm
```

Fibonacci program

```bash
./emulator run asm/fibonacci.asm
```
