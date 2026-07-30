#include <stdio.h>
#include <stdlib.h>

using Byte = unsigned char;
using Word = unsigned short;

using u32 = unsigned int;


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

   Word PC; //program counter
   Word SP; //stack pointer

   Byte A, X, Y; //registers

   Byte C : 1; //status flags
   Byte Z : 1;
   Byte I : 1;
   Byte D : 1;
   Byte B : 1;
   Byte V : 1;
   Byte N : 1;

   static constexpr Byte 
      INS_LDA_IM = 0xA9,
      INS_LDA_ZP = 0xA5,
      INS_LDA_ZPX = 0xB5,
      INS_JSR = 0x20;
      

   void LDASetStatus() {
      Z = (A == 0);
      N = (A & 0b10000000) > 0;
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
}