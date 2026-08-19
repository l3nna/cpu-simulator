#include <stdio.h>
#include <stdlib.h>

using Byte = unsigned char; // 8-bitni podatak (0 - 255)
using Word = unsigned short; // 16-bitni podatak (0 - 65535)
using u32 = unsigned int; // 32-bitni neoznačeni ceo broj


struct Memory {
   static constexpr u32 MAX_MEMORY_SIZE = 1024 * 64; // 64KB
   Byte Data[MAX_MEMORY_SIZE];

   void Initialise() {
      for(u32 i = 0; i < MAX_MEMORY_SIZE; i++) {
         Data[i] = 0;
      }
   }


   // read 1 byte
   Byte operator[](u32 Address) const {
      // assert here Address is < MAX_MEMORY_SIZE
      return Data[Address];
   }

   Byte& operator[](u32 Address) {
      // assert here Address is < MAX_MEMORY_SIZE
      return Data[Address];
   }

   void WriteWord
   (u32& Cycles, 
      Word Value, 
      u32 Address) 
   {
      Data[Address] = Value & 0xFF;
      Data[Address+1] = (Value >> 8);
      Cycles-=2;
   }
};


struct CPU {

   Word PC; // Program Counter (Pokazivač instrukcije)
   Word SP; // Stack Pointer (Pokazivač steka)

   Byte A, X, Y; // Opšti registri (Akumulator, X, Y)

   // Statusni flegovi (bit-fields od po 1 bit):
   Byte C : 1; // Carry (Prenos)
   Byte Z : 1; // Zero (Nula)
   Byte I : 1; // Interrupt Disable (Zabrana prekida)
   Byte D : 1; // Decimal Mode (Decimalni mod)
   Byte B : 1; // Break Command
   Byte V : 1; // Overflow (Prekoračenje)
   Byte N : 1; // Negative (Negativan broj)

   static constexpr Byte 
      INS_LDA_IM  = 0xA9, // LDA Immediate
      INS_LDA_ZP  = 0xA5, // LDA Zero Page
      INS_LDA_ZPX = 0xB5, // LDA Zero Page,X
      INS_JSR     = 0x20; // Jump to Subroutine
      

   void LDASetStatus() {
      Z = (A == 0);                   // Z fleg je 1 ako je Akumulator 0
      N = (A & 0b10000000) > 0;       // N fleg je 1 ako je 7. bit akumulatora (znak) jednak 1
   }

   void Reset(Memory& memory) {
      PC = 0xFFFC;
      SP = 0x0100;
      C = Z = I = D = B = V = N = 0;
      A = X = Y = 0;
      memory.Initialise();
   }

   void Execute(u32 Cycles, Memory& memory) {

      while (Cycles > 0) {
         Byte Instruction = FetchByte(Cycles, memory);

         switch (Instruction) {
            case INS_LDA_IM: {
               Byte Value = FetchByte(Cycles, memory);
               A = Value;
               LDASetStatus();
            } break;

            case INS_LDA_ZPX: {
               Byte ZeroPageAddress = FetchByte(Cycles, memory);
               ZeroPageAddress += X;
               Cycles--;
               A=ReadByte(Cycles, ZeroPageAddress, memory);
               LDASetStatus();
               
            } break;

            case INS_LDA_ZP: {
               Byte ZeroPageAddress = FetchByte(Cycles, memory);
               A = ReadByte(Cycles, ZeroPageAddress, memory);
               LDASetStatus();
            } break;

            case INS_JSR: {
               Word SubAddress = FetchWord(Cycles, memory);
               memory.WriteWord(Cycles, PC-1, SP);
               PC = SubAddress;
               Cycles--;
            }break;

            default:
            {
               printf("Unknown instruction: 0x%02X\n", Instruction);
            } break;
         }
      }

   }

   Byte FetchByte(u32 Cycles, Memory& memory) {
      Byte Data = memory[PC];
      PC++;
      Cycles--;
      return Data;
   }

   Word FetchWord(u32 Cycles, Memory& memory) {
      //6502 is little endian
      Word Data = memory[PC];
      PC++;
      Cycles--;

      Data = (memory[PC] << 8) | Data;
      PC++;
      Cycles+=2;

      //if u wanted to handle endianness, you could do it here, but the 6502 is little endian, so we don't need to.
      //if (PLATFORM_BIG_ENDIAN)
      // SwapBytesInWord(Data);
      return Data;
   }

   Byte ReadByte(
      u32 Cycles, 
      Word Address, 
      Memory& memory
   ) {
      Byte Data = memory[Address];
      Cycles--;
      return Data;
   }

};


int main() {
   Memory memory;
   CPU cpu;

   cpu.Reset(memory);

   //start - inline a little program
   memory[0xFFFC] = CPU::INS_JSR;
   memory[0xFFFD] = 0x42;
   memory[0xFFFE] = 0x42;
   memory[0x4242] = CPU::INS_LDA_IM;
   memory[0x4243] = 0x84;
   //end - inline a little program

   cpu.Execute(3, memory);

   return 0;

   printf("=== Stanje CPU-a nakon izvrsavanja ===\n");
   printf("Program Counter (PC) : 0x%04X\n", cpu.PC);
   printf("Registar A           : 0x%02X\n", cpu.A);
   printf("Negative Flag (N)    : %d\n", cpu.N);
   printf("Zero Flag (Z)        : %d\n", cpu.Z);

   return 0;
}


