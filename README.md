# LC3-VM

A virtual machine implementation that can run programs written in the [LC-3 (Little Computer 3)](https://en.wikipedia.org/wiki/Little_Computer_3) assembly language. It is written in C and simulates a 16-bit computer with a basic instruction set and a limited memory space.

## Features

- **Complete LC-3 Instruction Set**: Supports all LC-3 instructions including arithmetic, logic, control flow, and memory operations
- **Memory Management**: 16-bit address space with 65,536 memory locations
- **Register Operations**: 8 general-purpose registers, program counter, and condition flags
- **I/O Capabilities**: Terminal-based input/output through trap routines
- **Binary Image Loading**: Loads and executes LC-3 binary image files
- **Debug Features**: Register and memory inspection for troubleshooting

## Architecture

The LC-3 architecture includes:

- **Memory**: 16-bit address space (65,536 locations)
- **Registers**:
  - 8 general-purpose registers (R0-R7)
  - Program Counter (PC)
  - Condition Register (COND)
- **Instructions**: 16-bit instruction format with various operations
- **I/O**: Memory-mapped I/O for keyboard input and display output

## Building the VM

### Prerequisites

- C Compiler (GCC, Clang, etc.)
- CMake (version 3.10 or higher)
- Make

### Build Instructions

1. **Clone the repository**:
   ```bash
   git clone https://github.com/your-username/lc3-vm.git
   cd lc3-vm
   ```
2. **Create a build directory**:
    ```bash
    mkdir build
    cd build
    ```
3. **Configure with CMake**:
    ```bash
    cmake ..
    ```
4. **Build the project**:
    ```bash
    make
    ```
5. **Install (optional)**:
    ```bash
    make install
    ```

## Usage

After building the VM, you can run it with:

```bash
./lc3_vm [options] <image-file1> [image-file2 ...]
```

### Command-line Options

- `-h, --help`: Display help message
- `-d, --debug`: Enable debug mode
- `-m, --memory`: Dump memory before starting execution

## LC-3 Instruction Set

The VM supports all standard LC-3 instructions:

### Arithmetic and Logic

- `ADD` - Adds two values
- `AND` - Performs bitwise AND between two values
- `NOT` - Performs bitwise negation

### Control Flow

- `BR` - Branches to a new location based on condition flags
- `JMP` - Jumps to an address in a register
- `JSR` - Jumps to a subroutine and saves the return address

### Memory Operations

- `LD` - Loads data from memory
- `ST` - Stores data to memory
- `LDR` - Loads data from memory with register offset
- `STR` - Stores data to memory with register offset
- `LDI` - Loads data indirectly from memory
- `STI` - Stores data indirectly to memory
- `LEA` - Loads effective address

### System Operations

- `TRAP` - Executes system calls/functions