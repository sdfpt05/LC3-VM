# LC3-VM

This my attempt at building a virtual machine that can run assembly programs based on the [lc3](https://en.wikipedia.org/wiki/Little_Computer_3) computer.
It written in C and simulates a 16-bit computer with a basic instruction set and a limited memory space. The virtual machine loads a binary image and executes the instructions stored in memory.

## Resourses

- [Write your own virtual machine](https://www.jmeiners.com/lc3-vm/)
- [https://justinmeiners.github.io/lc3-vm/supplies/lc3-isa.pdf](https://justinmeiners.github.io/lc3-vm/supplies/lc3-isa.pdf)
- [LC3-Simulator](https://wchargin.com/lc3web/)

## Table of Contents

- [Features](#features)
- [Instruction Set](#instruction-set)
- [Installation](#installation)
- [Usage](#usage)
- [Trap Codes](#trap-codes)
- [Example](#example)
- [License](#license)

## Features

- Simulates a 16-bit processor with 8 general-purpose registers.
- Executes basic arithmetic, logic, control, and memory instructions.
- Supports input/output operations with terminal-based I/O.
- Handles memory-mapped registers for basic keyboard input simulation.
- Can load and run binary image files representing the program.

## Instruction Set

The virtual machine supports a set of operations, including:

- **Arithmetic and Logic**:

  - `ADD` - Adds two values.
  - `AND` - Performs bitwise AND between two values.
  - `NOT` - Performs bitwise negation.

- **Control**:

  - `BR` - Branches to a new location based on condition flags.
  - `JMP` - Jumps to a register's address.
  - `JSR` - Jumps to a subroutine and saves the return address.

- **Memory**:

  - `LD` - Loads data from memory.
  - `ST` - Stores data to memory.
  - `LDR`, `STR`, `LDI`, `STI`, `LEA` - Variations of load and store instructions.

- **Traps**:
  - Traps are special instructions for input/output operations, like reading from the keyboard, writing to the screen, etc.

## Installation

To install and run the VM simulator on your system, follow these steps:

1. **Clone the repository**:
   ```bash
   git clone https://github.com/your-username/virtual-machine-simulator.git
   cd virtual-machine-simulator
   ```
2. **Compile the Program**:

```bash

    gcc -o vm vm.c
```

    This will compile the vm.c file into an executable named vm.

## Usage

After compiling the program, you can run it using a binary image file. The binary image file should contain the machine code to be executed by the VM.

```bash
./vm <image-file1> <image-file2> ...
```

- The program expects at least one image file as input, which contains the binary instructions.
- Example:

```bash
   ./vm my_program.img
```

The virtual machine will read the binary file into memory, set the program counter to a default start location (0x3000), and begin executing instructions until it encounters a halt command or an invalid instruction.

## License

This project is licensed under the MIT License. See the LICENSE file for details.
