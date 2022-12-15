#ifndef EMULATOR_HPP
#define EMULATOR_HPP

#include <array>

#define SET_Z 0x0001   // or
#define CLEAR_Z 0xfffe // and
#define SET_O 0x0002
#define CLEAR_O 0xfffd
#define SET_C 0x0004
#define CLEAR_C 0xfffb
#define SET_N 0x0008
#define CLEAR_N 0xfff7

using namespace std;

class Emulator
{
private:
  array<short, 9> registers;
  array<unsigned char, 65536> memory;
  bool endFlag;

  string input_file;
  void printState();
  void executeInstruction();
  short resolveOperandAdding(unsigned char offsetCode, int regNum);
  short resolveAddressing(unsigned char offsetCode, unsigned char addressing, int regNum);
  void resolveStoreAddressing(unsigned char offsetCode, unsigned char addressing, int regD, int regS);
  void setData();
public:
  Emulator(string input);
  void Emulate();
};

#endif