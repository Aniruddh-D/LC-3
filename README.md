# LC-3 Simulator

## 🔍 Overview
The LC-3 Simulator is a project inspired by [Justin Meiners' LC-3 VM](https://justinmeiners.github.io/lc3-vm/). The LC-3 (Little Computer 3) is a simple educational computer architecture designed to teach the fundamentals of computer organization and assembly programming.

This simulator provides an emulation environment for executing LC-3 assembly programs, supporting a complete set of LC-3 instructions, memory, registers, and system calls.

## 🌟 Features
- ✅ **Complete LC-3 Instruction Set**: Supports all 16 opcodes defined in the LC-3 ISA.
- 🏗 **Registers and Memory**:
  - 8 general-purpose registers (R0-R7).
  - 1 program counter (PC).
  - 1 condition flag register (COND) with POS (Positive), ZRO (Zero), and NEG (Negative) flags.
  - 65536 memory locations (16-bit addresses).
- ⚡ **Interrupt Handling**: Supports interrupts using signal handling mechanisms.
- 🎛 **I/O Support**: Implements TRAP routines for basic I/O operations (GETC, OUT, PUTS, etc.).
- 🖥 **Cross-Platform Compatibility**:
  - Works on **Windows (MinGW)** and **UNIX-based systems (Linux/macOS)**.
  - Uses platform-specific libraries for keyboard input handling.
- 🎮 **Play 2048 and Rogue**: The simulator supports running LC-3 assembly games like **2048** and **Rogue**.

## 🏗 LC-3 Instruction Set
The LC-3 supports the following 16 instructions:

| 🔢 Opcode | 📝 Instruction | 🛠 Description |
|---------|------------|-------------|
| 0001    | ADD        | Addition (register/immediate) |
| 0101    | AND        | Bitwise AND (register/immediate) |
| 0000    | BR         | Conditional branch |
| 1100    | JMP        | Unconditional jump |
| 0100    | JSR/JSRR   | Jump to subroutine |
| 0010    | LD         | Load memory into register |
| 1010    | LDI        | Load indirect |
| 0110    | LDR        | Load register |
| 1110    | LEA        | Load effective address |
| 1001    | NOT        | Bitwise NOT |
| 1000    | RTI        | Return from interrupt |
| 0011    | ST         | Store register into memory |
| 1011    | STI        | Store indirect |
| 0111    | STR        | Store register |
| 1111    | TRAP       | System call |

## 🛠 Compilation and Setup
### 🖥 Windows
Ensure you have MinGW installed, then compile using:
```sh
gcc lc-3.c -o lc3.exe
```
### 🖥 UNIX (Linux/macOS)
Ensure `gcc` is installed, then compile using:
```sh
gcc lc-3.c -o lc3
```

## ▶️ Running the Simulator
To run the LC-3 simulator, provide an LC-3 binary image as input:
```sh
./lc3 [image-file]
```
Example:
```sh
./lc3 test.obj
```

## 🖥 System Calls (TRAP Routines)
TRAP routines provide input and output operations to interact with the user.

| 🎯 TRAP Code | 🎭 Function |
|-----------|----------|
| 0x20      | GETC (Get character from keyboard, not echoed) |
| 0x21      | OUT (Output a character) |
| 0x22      | PUTS (Output a string) |
| 0x23      | IN (Get character from keyboard, echoed) |
| 0x24      | PUTSP (Output a byte string) |
| 0x25      | HALT (Halt execution) |

## 🎮 Example Usage
### 📜 Running a Test Program
If you have an assembled LC-3 binary (e.g., `test.obj`), you can run it as follows:
```sh
./lc3 test.obj
```

### ✍️ Sample Assembly Code (Hello World)
Below is an example LC-3 assembly program that prints "Hello, World!":
```assembly
.ORIG x3000
LEA R0, HELLO
PUTS
HALT
HELLO .STRINGZ "Hello, World!"
.END
```
To assemble and run it, use an LC-3 assembler like `lc3as` and provide the generated object file to the simulator.

## 🛠 Debugging and Development
- **Modify Memory or Registers**: The code includes `print_state()` to output register and memory values for debugging.
- **Handling Interrupts**: Uses `signal(SIGINT, handle_interrupt);` to restore input buffering upon termination.
- **Platform-Specific Input Handling**:
  - 🖥 **Windows**: Uses `_kbhit()` and `WaitForSingleObject()`.
  - 🖥 **UNIX**: Uses `select()` with `STDIN_FILENO`.

## 📜 License
This project is open-source and free to use. Inspired by the work of Justin Meiners.

## 👨‍💻 Author
**Aniruddh-D**
