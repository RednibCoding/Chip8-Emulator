#include "chip8.h"


void chip8Init(Chip8 *chip8)
{
    chip8Reset(chip8);
}

void chip8Reset(Chip8 *chip8)
{
    // Reset memory
    for (int m = 0; m < MEM_SIZE; m++)
    {
        chip8->memory[m] = 0;
    }

    // Reset registers
    for (int v = 0; v < NUM_REGISTERS; v++)
    {
        chip8->V[v] = 0;
    }

    // Reset stack
    for (int s = 0; s < STACK_SIZE; s++)
    {
        chip8->stack[s] = 0;
    }

    chip8->sp = 0;
    chip8->st = 0;
    chip8->dt = 0;
    chip8->I = 0;
    chip8->pc = START_ADDRESS;

    chip8->drawFlag = false;
    chip8->hlt = false;
}

bool chip8IsDrawFlagSet(Chip8 *chip8)
{
    return chip8->drawFlag;
}

void chip8EmulateCycle(Chip8 *chip8)
{
    if (!chip8->hlt)
    {
        unsigned short opcode = chip8FetchOpcode(chip8);
        chip8HandleOpcode(chip8, opcode);
        chip8HandleTimers(chip8);
    }
}

unsigned short chip8FetchOpcode(Chip8 *chip8)
{
    /*
        One instruction consists of 2 bytes (16 bit).
        However memory stores only 1 byte values (8 bit).
        So a full instruction spreads over 2 memory locations (pc and pc+1).
        So both have to be fetched and combined (ored)
        Example:
            pc:     00000000 11000011
            pc+1:   00000000 00001100

            Shifting pc 8 bits to the left:
            pc:     11000011 00000000
            pc+1:   00000000 00001100

            pc ored with pc+1
                    pc:     11000011 00000000
                    pc+1:   00000000 00001100
                    ored:   11000011 00001100
    */
    unsigned short opcode = chip8->memory[chip8->pc] << 8 | chip8->memory[chip8->pc + 1];
    return opcode;
}

void chip8HandleTimers(Chip8 *chip8)
{
    if (chip8->dt > 0)
    {
        chip8->dt--;
    }

    if (chip8->st > 0)
    {
        if (chip8->st == 1)
        {
            // Perform BEEP sound
        }
        chip8->st--;
    }
}

void chip8HandleOpcode(Chip8 *chip8, unsigned short opcode)
{
    // Look at the first 4 bits to determine what instruction it is
    switch (opcode & 0xF000)
    {
        // Opcode starts with 0x0... (there are more than one instruction starting with 0x0)
        case 0x0000:
            // Look at the last 4 bits
            switch (opcode & 0x000F)
            {
                // CLS - 0x00E0: Clear Screen
                case 0x0000: 
                    // TODO: Clear the screen
                    break;

                // RET - 0x00EE: Return from subroutine
                case 0x000E:
                    if (chip8->sp < 0)
                    {
                        chip8->hlt = true;
                        printf("Stack underflow while executing: 0x%x\n", opcode);
                        break;
                    }
                    chip8->pc = chip8->stack[chip8->sp];
                    chip8->sp--;
                    break;

                default:
                    chip8->hlt = true;
                    printf("Unknown opcode: 0x%x\n", opcode);
                    break;
            }
            break;

        // JP addr - 1NNN: Jump to location NNN
        case 0x1000:
            chip8->pc = opcode & 0x0FFF;
            break;

        // CALL addr - 0x2NNN: Call subroutine at address NNN
        case 0x2000:
            if (chip8->sp == STACK_SIZE-1)
            {
                chip8->hlt = true;
                printf("Stack overflow while executing: 0x%x\n", opcode);
                break;
            }

            chip8->sp++;
            // Save return address on the stack
            chip8->stack[chip8->sp] = chip8->pc + INSTRUCTION_SIZE;
            chip8->pc = opcode & 0x0FFF;
            break;

        // SE VX, NN - 3XNN: Skip next instruction if VX == NN
        case 0x3000:
            if (chip8->V[(opcode & 0x0F00) >> 8] == opcode & 0x00FF)
                chip8->pc += INSTRUCTION_SIZE * 2; // Skip next instruction
            else
                chip8->pc += INSTRUCTION_SIZE; // Goto next instruction
            break;

        // SNE VX, NN - 4XNN: Skip next instruction if VX != NN
        case 0x4000:
            if (chip8->V[(opcode & 0x0F00) >> 8] != opcode & 0x00FF)
                chip8->pc += INSTRUCTION_SIZE * 2; // Skip next instruction
            else
                chip8->pc += INSTRUCTION_SIZE; // Goto next instruction
            break;

        // SE VX, VY - 5XY0: Skip next instruction if VX == VY
        case 0x5000:
            if (chip8->V[(opcode & 0x0F00) >> 8] == chip8->V[(opcode & 0x00F0) >> 4])
                chip8->pc += INSTRUCTION_SIZE * 2; // Skip next instruction
            else
                chip8->pc += INSTRUCTION_SIZE; // Goto next instruction
            break;

        // LD VX, NN - 6XNN: Set VX to NN
        case 0x6000:
            chip8->V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
            chip8->pc += INSTRUCTION_SIZE;
            break;

        // ADD VX, NN - 7XNN: Set VX = VX + NN (no carry generated)
        case 0x7000:
            chip8->V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
            chip8->pc += INSTRUCTION_SIZE;
            break;

        // Opcode starts with 0x8... (there are more than one instruction starting with 0x8)
        case 0x8000:
            // Look at the last 4 bits
            switch (opcode & 0x000F)
            {
                // LD VX, VY - 8XY0: Set VX = VY
                case 0x0000:
                    chip8->V[(opcode & 0x0F00) >> 8] = chip8->V[(opcode & 0x00F0) >> 4];
                    chip8->pc += INSTRUCTION_SIZE;
                    break;

                // OR VX, VY - 8XY1: Set VX = VX OR VY
                case 0x0001:
                    chip8->V[(opcode & 0x0F00) >> 8] |= chip8->V[(opcode & 0x00F0) >> 4];
                    chip8->pc += INSTRUCTION_SIZE;
                    break;

                // AND VX, VY - 8XY2: Set VX = VX AND VY
                case 0x0002:
                    chip8->V[(opcode & 0x0F00) >> 8] &= chip8->V[(opcode & 0x00F0) >> 4];
                    chip8->pc += INSTRUCTION_SIZE;
                    break;

                // XOR VX, VY - 8XY2: Set VX = VX XOR VY
                case 0x0003:
                    chip8->V[(opcode & 0x0F00) >> 8] ^= chip8->V[(opcode & 0x00F0) >> 4];
                    chip8->pc += INSTRUCTION_SIZE;
                    break;

                // ADD VX, VY - 8XY4: Set VX = VX + VY   set VF = carry
                case 0x0004:
                    chip8->V[(opcode & 0x0F00) >> 8] += chip8->V[(opcode & 0x00F0) >> 4];
                    if (chip8->V[(opcode & 0x0F00) >> 8] + chip8->V[(opcode & 0x00F0) >> 4] > 0xFF)
                        chip8->V[0xF] = 1;
                    chip8->V[0xF] = 0;
                    chip8->pc += INSTRUCTION_SIZE;
                    break;

                // SUB VX, VY - 8XY5: Set VX = VX - VY   set VF = NOT borrow
                case 0x0005:
                    if (chip8->V[(opcode & 0x0F00) >> 8] > chip8->V[(opcode & 0x00F0) >> 4])
                        chip8->V[0xF] = 1;
                    else
                        chip8->V[0xF] = 0;
                    
                    chip8->V[(opcode & 0x0F00) >> 8] -= chip8->V[(opcode & 0x00F0) >> 4];
                    chip8->pc += INSTRUCTION_SIZE;
                    break;

                // SHR VX - 8XY6: Set VX = VX SHR 1   (Y is not used)
                // If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0.
                case 0x0006:
                    chip8->V[0xF] = chip8->V[(opcode & 0x0F00) >> 8] & 1;
                    chip8->V[(opcode & 0x0F00) >> 8] >>= 1;
                    chip8->pc += INSTRUCTION_SIZE;
                    break;

                // SUBN VX, VY - 8XYN: Set VX = VY - VX   set VF = NOT borrow
                case 0x0007:
                    if (chip8->V[(opcode & 0x00F0) >> 4] > chip8->V[(opcode & 0x0F00) >> 8])
                        chip8->V[0xF] = 1;
                    chip8->V[0xF] = 0;

                    chip8->V[(opcode & 0x0F00) >> 8] = chip8->V[(opcode & 0x00F0) >> 4] - chip8->V[(opcode & 0x0F00) >> 8];
                    chip8->pc += INSTRUCTION_SIZE;
                    break;

                // SHL VX - 8XYE: Set VX = VX SHL 1   (Y is not used)
                // If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0.
                case 0x000E:
                    chip8->V[0xF] = chip8->V[(opcode & 0x0F00) >> 8] >> 7;

                    chip8->V[(opcode & 0x0F00) >> 8] <<= 1;
                    chip8->pc += INSTRUCTION_SIZE;
                    break;

                default:
                    chip8->hlt = true;
                    printf("Unknown opcode: 0x%x\n", opcode);
                    break;
            }
            break;
        
        // SNE VX, VY - 9XY0: Skip next instruction if VX != VY
        case 0x9000:
            if (chip8->V[(opcode & 0x0F00) >> 8] != chip8->V[(opcode & 0x00F0) >> 4])
                chip8->pc += INSTRUCTION_SIZE * 2;
            else
                chip8->pc += INSTRUCTION_SIZE;
            break;

        // LD I addr - ANNN: Sets I to address NNN
        case 0xA000:
            chip8->I = opcode & 0x0FFF;
            chip8->pc += INSTRUCTION_SIZE;
            break;

        default:
            printf("Unknown opcode: 0x%x\n", opcode);
            break;
    }
}

void chip8DrawGraphics(Chip8 *chip8)
{
    if (!chip8->hlt)
    {
        // TODO: Draw stuff
    }
}