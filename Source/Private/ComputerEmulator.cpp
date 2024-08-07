// ComputerEmulator.cpp : Defines the entry point for the application.
//

#include "../Public/ComputerEmulator.h"

using namespace std;
using Byte = unsigned char;
using Word = unsigned short;
using u32 = unsigned int;

struct Mem {

	static constexpr u32 MAX_MEM = 1024 * 64;
	Byte Data[MAX_MEM];

	void Initialize() {
		for (u32 i = 0; i < MAX_MEM; i++) {
			Data[i] = 0;
		}
	}

	//Reading 1 byte
	Byte operator[] (u32 Address) const {

		return Data[Address];
	}

	//Writing 1 byte
	Byte& operator[] (u32 Address) {

		return Data[Address];
	}

	void WriteWord( u32& Cycles, Word Value, u32 Address) {
		Data[Address] = Value & 0xFF;
		Data[Address + 1] = (Value >> 8);
		Cycles -= 2;
	}
};

struct CPU {
	/*
		In order:
		Program counter, stack pointer, register and status flags definition
	*/

	Word PC;
	Byte SP;

	Byte A, X, Y; 

	Byte C : 1;
	Byte Z : 1;
	Byte I : 1;
	Byte D : 1;
	Byte B : 1;
	Byte V : 1;
	Byte N : 1;


	void Reset(Mem& memory) {

		PC = 0xFFFC;
		SP = 0x0100;
		D = 0;
		C = Z = I = D = B = V = N = 0;
		A = X = Y = 0;
		memory.Initialize();
	}

	Byte FetchByte(u32 Cycles, Mem& memory) {
		Byte Data = memory[PC];
		PC++;
		Cycles--;
		return Data;

	}

	Word FetchWord(u32 Cycles, Mem& memory) {

		Word Data = memory[PC];
		PC++;
		
		

		Data |= (memory[PC] << 8);
		PC++;


		Cycles -= 2;

		return Data;
	}

	Byte ReadByte(u32& Cycles, Byte Address, Mem& memory) {
		Byte Data = memory[Address];
		Cycles--;
		return Data;
	}

	static constexpr Byte
		INS_LDA_IM = 0xA9,
		INS_LDA_ZP = 0xA5,
		INS_LDA_ZPX = 0xB5,
		INS_JSR = 0x20;

	void LDASetStatus() {
		Z = (A == 0);
		N = (A & 0b10000000) > 0;
	}

	void Execute(u32 Cycles, Mem& memory) {

		while (Cycles > 0) {
			Byte Instruction = FetchByte(Cycles, memory);
			switch (Instruction) {
			case INS_LDA_IM:{

				Byte Value = FetchByte(Cycles, memory);
				A = Value;
				LDASetStatus();
			}break;

			case INS_LDA_ZP: {

				Byte ZeroPageAddress = FetchByte(Cycles, memory);
				A = ReadByte(Cycles, ZeroPageAddress, memory);

				LDASetStatus();


			}break;

			case INS_LDA_ZPX: {
				Byte ZeroPageAddress = FetchByte(Cycles, memory);
				ZeroPageAddress += X;
				Cycles--;
				A = ReadByte(Cycles, ZeroPageAddress, memory);

				LDASetStatus();

			}break;
			case INS_JSR:
			{
				Word SubAddr = FetchWord(Cycles, memory);
				memory.WriteWord( PC - 1, SP, Cycles);
				PC = SubAddr;
				Cycles--;
			}break;
			default: {
				printf("Instruction not handled");
			}

			}
		}
	}

};

int main()
{
	Mem mem;
	CPU cpu;
	cpu.Reset(mem);
	mem[0xFFFC] = CPU::INS_JSR;
	mem[0xFFFD] = 0x42;
	mem[0xFFFE] = 0x42;
	mem[0x4242] = CPU::INS_LDA_IM;
	mem[0x4243] = 0x84;
	cpu.Execute( 9, mem);
	return 0;
}
