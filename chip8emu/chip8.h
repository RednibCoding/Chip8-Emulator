#include <stdbool.h>


#ifndef CHIP8_H
#define CHIP8_H


/*
 * Chip8
 *
 * The main Chip 8 CPU. All logic and instructions live within
 * the CPU.
 */


#define MEM_SIZE 4096
#define NUM_REGISTERS 16
#define STACK_SIZE 16
// An instruction is 2 bytes (16 bit)
#define INSTRUCTION_SIZE 2
// Start address of program (rom) is 0x200
#define START_ADDRESS 0x200


typedef struct
{
    // Chip 8 has memory of 4 kilobytes
    unsigned char memory[MEM_SIZE];

    // Chip 8 has 16 8-bit registers named 'V'
    // Register V0 to V14 are general purpose
    // Register V15 is Cary Flag
    unsigned char V[NUM_REGISTERS];

    // Stack of a Chip 8 can store up to 16 2-byte addresses
    unsigned short stack[STACK_SIZE];

    // Stack pointer
    unsigned char sp;

    // Sound timer
    unsigned char st;

    // Delay timer
    unsigned char dt;

    // Index register
    unsigned short I;

    // Programm counter
    unsigned short pc;

    // Draw flag
    bool drawFlag;

    // Halted flag
    bool hlt;

} Chip8;

void chip8Init(Chip8 *chip8);
void chip8Reset(Chip8 *chip8);
bool chip8IsDrawFlagSet(Chip8 *chip8);
void chip8EmulateCycle(Chip8 *chip8);
void chip8HandleOpcode(Chip8 *chip8, unsigned short opcode);
void chip8HandleTimers(Chip8 *chip8);
void chip8DrawGraphics(Chip8 *chip8);

#endif // CHIP8_H
