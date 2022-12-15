#include "../inc/Emulator.hpp"
#include <fstream>
#include <regex>
#include <iostream>

Emulator::Emulator(string input)
{
  input_file = input;
  registers = array<short, 9>();
  for (int i = 0; i < 9; i++)
  {
    registers[i] = 0;
  }
  memory = array<unsigned char, 65536>();
  for (int i = 0; i < 65536; i++)
  {
    memory[i] = 0;
  }
  this->setData();
  registers[6] = 65279; // memory under last 256B which are reserved for mem.mapped registers
  registers[7] = memory[1];
  registers[7] <<= 8;
  registers[7] += memory[0];
  endFlag = false;
}

void Emulator::Emulate()
{
  while (!endFlag)
  {
    this->executeInstruction();

    //FOR FINDING ERRORS
    //this->printState();
  }
  this->printState();
}

void Emulator::printState()
{
  cout << "\n------------------------------------------------" << endl;
  cout << "Emulated processor executed halt instruction" << endl;
  bitset<16> psw(registers[8]);
  cout << "Emulated processor state: psw= 0b" << psw << endl;
  cout << "r0=" << hex << registers[0] << " r1=" << hex << registers[1] << " r2="
       << hex << registers[2] << " r3=" << hex << registers[3] << endl;
  cout << "r4=" << hex << registers[4] << " r5=" << hex << registers[5] << " r6="
       << hex << registers[6] << " r7=" << hex << registers[7] << endl;

  //FOR FINDING ERRORS 
  /*cout<<"\n\n STEK:"<<endl;
  for (int i = 0xfede; i < 0xfefe; i++)
  {
    cout << hex << static_cast<int>(memory[i])<<" ";

  }
  cout<<"\n";
  for (int i = 0x12a; i < 0x138; i+=2)
  {
    cout <<"value"<<((i-0x12a)/2)<<":"<< hex <<static_cast<int>(memory[i+1])<<" "
      <<hex <<static_cast<int>(memory[i])<<endl;

  }
  cout<<"\n\n";
  
  pazi kako pristupas memoriji, indeks mora biti tipa unsigned char!!!
  */
}

void Emulator::executeInstruction()
{
  unsigned char instruction_code = memory[registers[7]];

  //TRACE
  //cout << hex << static_cast<int>(instruction_code) << endl;

  switch (instruction_code)
  {
  case 0x00: // halt
  {
    endFlag = true;
    registers[7]+=1;
    break;
  }
  case 0x10: // int
  {
    short pc = registers[7];
    int regNum = (memory[pc + 1] >> 4) & 0x0f;
    if (regNum > 8 || (memory[pc + 1] & 0x0f) != 0x0f)
    {
      cerr << "ERROR[-13] Emulator error: Illegal instruction code [int]!" << endl;
      exit(-13);
    }

    unsigned short sp = (unsigned short)registers[6];

    // push pc
    registers[6] -= 2;
    sp-=2;
    unsigned short next_instruction = registers[7] + 2;
    memory[sp + 1] = (unsigned char)(next_instruction >> 8);
    memory[sp] = (unsigned char)next_instruction;

    // push psw
    registers[6] -= 2;
    sp-=2;
    memory[sp + 1] = (unsigned char)(registers[8] >> 8);
    memory[sp] = (unsigned char)(registers[8]);

    registers[7] = (memory[(registers[regNum] % 8) * 2 + 1]<<8) + memory[(registers[regNum] % 8) * 2];
    break;
  }
  case 0x20: // iret
  {
    unsigned short sp = registers[6];

    // pop psw
    registers[8] = (((short)memory[sp + 1]) << 8) + memory[sp];
    
    // pop pc
    registers[7] = memory[sp + 2] + (((short)memory[sp + 3]) << 8);

    registers[6] += 4;
    break;
  }
  case 0x30: // call
  {
    short pc = registers[7];
    int regNum = memory[pc + 1] & 0x0f;
    int trash = (memory[pc + 1] >> 4) & 0x0f;
    unsigned char offsetCode = (memory[pc + 2] >> 4) & 0x0f;
    unsigned char memoryAddressing = memory[pc + 2] & 0x0f;
    if (regNum > 8 || regNum < 0 || trash != 0x0f || offsetCode > 4 ||
        offsetCode < 0 || memoryAddressing < 0 || memoryAddressing > 5)
    {
      cerr << "ERROR[-13] Emulator error: Illegal instruction code [call]!" << endl;
      exit(-13);
    }
    // push pc
    registers[6] -= 2;
    unsigned short sp = registers[6];
    short next_instruction = registers[7];
    if (memoryAddressing == 1 || memoryAddressing == 2)
      next_instruction += 3;
    else
      next_instruction += 5;

    memory[sp+1] = (unsigned char)(next_instruction >> 8);
    memory[sp] = (unsigned char)next_instruction;
    

    // pc = operand;
    registers[7] = this->resolveAddressing(offsetCode, memoryAddressing, regNum);

    break;
  }
  case 0x40: // ret
  {
    unsigned short sp = registers[6];

    // pop pc
    registers[7] = memory[sp] + (((unsigned short)memory[sp + 1]) << 8);
    registers[6] += 2;
    break;
  }
  case 0x50: // jmp
  {
    short pc = registers[7];
    int trash = (memory[pc + 1] >> 4) & 0x0f;
    int regNum = memory[pc + 1] & 0x0f;
    unsigned char memoryAddressing = memory[pc + 2] & 0x0f;
    unsigned char offsetCode = (memory[pc + 2] >> 4) & 0x0f;
    if (regNum > 8 || regNum < 0 || trash != 0x0f || offsetCode > 4 || memoryAddressing > 5)
    {
      cerr << "ERROR[-13] Emulator error: Illegal instruction code [jmp]!" << endl;
      exit(-13);
    }

    registers[7] = this->resolveAddressing(offsetCode, memoryAddressing, regNum);
    break;
  }
  case 0x51: // jeq
  {
    short pc = registers[7];
    int trash = (memory[pc + 1] >> 4) & 0x0f;
    int regNum = memory[pc + 1] & 0x0f;
    unsigned char memoryAddressing = memory[pc + 2] & 0x0f;
    unsigned char offsetCode = (memory[pc + 2] >> 4) & 0x0f;
    if (regNum > 8 || regNum < 0 || trash != 0x0f || offsetCode > 4 || memoryAddressing > 5)
    {
      cerr << "ERROR[-13] Emulator error: Illegal instruction code [jeq]!" << endl;
      exit(-13);
    }
    if (registers[8] & 0x0001 == 1)
      registers[7] = this->resolveAddressing(offsetCode, memoryAddressing, regNum);
    else
    {
      if (memoryAddressing == 1 || memoryAddressing == 2)
        registers[7] += 3;
      else
        registers[7] += 5;
    }
    break;
  }
  case 0x52: // jne
  {
    short pc = registers[7];
    int trash = (memory[pc + 1] >> 4) & 0x0f;
    int regNum = memory[pc + 1] & 0x0f;
    unsigned char memoryAddressing = memory[pc + 2] & 0x0f;
    unsigned char offsetCode = (memory[pc + 2] >> 4) & 0x0f;
    if (regNum > 8 || regNum < 0 || trash != 0x0f || offsetCode > 4 || memoryAddressing > 5)
    {
      cerr << "ERROR[-13] Emulator error: Illegal instruction code [jne]!" << endl;
      exit(-13);
    }
    if ((registers[8] & 0x0001) == 0)
      registers[7] = this->resolveAddressing(offsetCode, memoryAddressing, regNum);
    else
    {
      if (memoryAddressing == 1 || memoryAddressing == 2)
        registers[7] += 3;
      else
        registers[7] += 5;
    }
    break;
  }
  case 0x53: // jgt
  {
    short pc = registers[7];
    int trash = (memory[pc + 1] >> 4) & 0x0f;
    int regNum = memory[pc + 1] & 0x0f;
    unsigned char memoryAddressing = memory[pc + 2] & 0x0f;
    unsigned char offsetCode = (memory[pc + 2] >> 4) & 0x0f;
    if (regNum > 8 || regNum < 0 || trash != 0x0f || offsetCode > 4 || memoryAddressing > 5)
    {
      cerr << "ERROR[-13] Emulator error: Illegal instruction code [jgt]!" << endl;
      exit(-13);
    }
    if (registers[8] & 0x0001 == 1 && registers[8] & 0x0008 == 0)
      registers[7] = this->resolveAddressing(offsetCode, memoryAddressing, regNum);
    else
    {
      if (memoryAddressing == 1 || memoryAddressing == 2)
        registers[7] += 3;
      else
        registers[7] += 5;
    }
    break;
  }
  case 0x60:
  {
    int pc = registers[7];
    int reg1 = (memory[pc + 1] >> 4) & 0x0f;
    int reg2 = memory[pc + 1] & 0x0f;

    // swap values
    short tmp = registers[reg1];
    registers[reg1] = registers[reg2];
    registers[reg2] = tmp;

    // next instruction
    registers[7] += 2;
    break;
  }
  case 0x70: // add
  {
    int pc = registers[7];
    int reg1 = (memory[pc + 1] >> 4) & 0x0f;
    int reg2 = memory[pc + 1] & 0x0f;

    registers[reg1] = registers[reg1] + registers[reg2];

    // next instruction
    registers[7] += 2;
    break;
  }
  case 0x71: // sub
  {
    int pc = registers[7];
    int reg1 = (memory[pc + 1] >> 4) & 0x0f;
    int reg2 = memory[pc + 1] & 0x0f;

    registers[reg1] = registers[reg1] - registers[reg2];

    // next instruction
    registers[7] += 2;
    break;
  }
  case 0x72: // mull
  {
    int pc = registers[7];
    int reg1 = (memory[pc + 1] >> 4) & 0x0f;
    int reg2 = memory[pc + 1] & 0x0f;

    registers[reg1] = registers[reg1] * registers[reg2];

    // next instruction
    registers[7] += 2;
    break;
  }
  case 0x73: // div
  {
    int pc = registers[7];
    int reg1 = (memory[pc + 1] >> 4) & 0x0f;
    int reg2 = memory[pc + 1] & 0x0f;

    registers[reg1] = registers[reg1] / registers[reg2];

    // next instruction
    registers[7] += 2;
    break;
  }
  case 0x74: // cmp
  {
    int pc = registers[7];
    int reg1 = (memory[pc + 1] >> 4) & 0x0f;
    int reg2 = memory[pc + 1] & 0x0f;

    int operand1 = registers[reg1];
    int operand2 = registers[reg2];
    if (operand1 == operand2)
    {
      registers[8] |= SET_Z;
      registers[8] &= CLEAR_N;
      registers[8] &= CLEAR_C;
      registers[8] &= CLEAR_O;
    }
    else if (operand1 < operand2)
    {
      registers[8] &= CLEAR_Z;
      registers[8] |= SET_C;
      if (operand1 - operand2 > 0xffff0000) // overflow
        registers[8] |= SET_O;
      else
        registers[8] &= CLEAR_O;
      registers[8] |= SET_N;
    }
    else
    {
      registers[8] &= CLEAR_Z;
      registers[8] &= CLEAR_C;
      registers[8] &= CLEAR_O;
      registers[8] &= CLEAR_N;
    }
    // next instruction
    registers[7] += 2;
    break;
  }
  case 0x80: // not
  {
    int pc = registers[7];
    int reg1 = (memory[pc + 1] >> 4) & 0x0f;
    int reg2 = memory[pc + 1] & 0x0f;

    registers[reg1] = ~registers[reg1];

    // next instruction
    registers[7] += 2;
    break;
  }
  case 0x81: // and
  {
    int pc = registers[7];
    int reg1 = (memory[pc + 1] >> 4) & 0x0f;
    int reg2 = memory[pc + 1] & 0x0f;

    registers[reg1] = registers[reg1] & registers[reg2];

    // next instruction
    registers[7] += 2;
    break;
  }
  case 0x82: // or
  {
    int pc = registers[7];
    int reg1 = (memory[pc + 1] >> 4) & 0x0f;
    int reg2 = memory[pc + 1] & 0x0f;

    registers[reg1] = registers[reg1] | registers[reg2];

    // next instruction
    registers[7] += 2;
    break;
  }
  case 0x83: // xor
  {
    int pc = registers[7];
    int reg1 = (memory[pc + 1] >> 4) & 0x0f;
    int reg2 = memory[pc + 1] & 0x0f;

    registers[reg1] = registers[reg1] ^ registers[reg2];

    // next instruction
    registers[7] += 2;
    break;
  }
  case 0x84: // test
  {
    int pc = registers[7];
    int reg1 = (memory[pc + 1] >> 4) & 0x0f;
    int reg2 = memory[pc + 1] & 0x0f;

    int temp = registers[reg1] & registers[reg2];
    if (temp == 0)
    {
      registers[8] |= SET_Z;
      registers[8] &= CLEAR_N;
    }
    else if (temp > 32768)
    {
      registers[8] &= CLEAR_Z;
      registers[8] |= SET_N;
    }
    else
    {
      registers[8] &= CLEAR_Z;
      registers[8] &= CLEAR_N;
    }

    // next instruction
    registers[7] += 2;
    break;
  }
  case 0x90: // shl
  {
    int pc = registers[7];
    int reg1 = (memory[pc + 1] >> 4) & 0x0f;
    int reg2 = memory[pc + 1] & 0x0f;

    if (registers[reg2] > 0)
    {
      registers[reg1] <<= (registers[reg2] - 1);
      if ((registers[reg1] & 0x8000) != 0)
        registers[8] |= SET_C;
      else
        registers[8] &= CLEAR_C;
      registers[reg1] <<= 1;
      if ((registers[reg1] & 0x8000) != 0)
        registers[8] |= SET_N;
      else
        registers[8] &= CLEAR_N;
      if (registers[reg1] == 0)
        registers[8] |= SET_Z;
      else
        registers[8] &= CLEAR_Z;
    }

    // next instruction
    registers[7] += 2;
    break;
  }
  case 0x91: // shr
  {
    int pc = registers[7];
    int reg1 = (memory[pc + 1] >> 4) & 0x0f;
    int reg2 = memory[pc + 1] & 0x0f;
    if (registers[reg2] > 0)
    {
      registers[reg1] >>= (registers[reg2] - 1);
      if ((registers[reg1] & 0x0001) != 0)
        registers[8] |= SET_C;
      else
        registers[8] &= CLEAR_C;
      registers[reg1] >>= 1;
      if ((registers[reg1] & 0x8000) != 0)
        registers[8] |= SET_N;
      else
        registers[8] &= CLEAR_N;
      if (registers[reg1] == 0)
        registers[8] |= SET_Z;
      else
        registers[8] &= CLEAR_Z;
    }

    // next instruction
    registers[7] += 2;
    break;
  }
  case 0xa0: // ldr
  {
    short pc = registers[7];
    unsigned char regD = (memory[pc + 1] >> 4) & 0x0f;
    unsigned char regS = memory[pc + 1] & 0x0f;
    unsigned char memoryAddressing = memory[pc + 2] & 0x0f;
    unsigned char offsetCode = (memory[pc + 2] >> 4) & 0x0f;
    if (regS > 8 || regD > 8 || offsetCode > 4 || memoryAddressing > 5)
    {
      cerr << "ERROR[-13] Emulator error: Illegal instruction code [ldr]!" << endl;
      exit(-13);
    }

    // load operand
    registers[regD] = this->resolveAddressing(offsetCode, memoryAddressing, regS);
    break;
  }
  case 0xb0: // str
  {
    short pc = registers[7];
    unsigned char regD = (memory[pc + 1] >> 4) & 0x0f;
    unsigned char regS = memory[pc + 1] & 0x0f;
    unsigned char memoryAddressing = memory[pc + 2] & 0x0f;
    unsigned char offsetCode = (memory[pc + 2] >> 4) & 0x0f;
    if (regS > 8 || regD > 8 || offsetCode > 4 || memoryAddressing > 5)
    {
      cerr << "ERROR[-13] Emulator error: Illegal instruction code [str]!" << endl;
      exit(-13);
    }

    // store to operand
    this->resolveStoreAddressing(offsetCode, memoryAddressing, regD, regS);
    break;
  }

  default:
  {
    cerr << "ERROR[-13] Emulator error: Illegal instruction code!" << endl;
    exit(-13);
    break;
  }
  }
}

short Emulator::resolveOperandAdding(unsigned char offsetCode, int regNum)
{
  int operand = 0;
  switch (offsetCode)
  {
  case 0:
    operand = registers[regNum];
    break;
  case 1:
    registers[regNum] -= 2;
    operand = registers[regNum];
    break;
  case 2:
    registers[regNum] += 2;
    operand = registers[regNum];
    break;
  case 3:
    operand = registers[regNum];
    registers[regNum] -= 2;
    break;
  case 4:
    operand = registers[regNum];
    registers[regNum] += 2;
    break;

  default:
    break;
  }
  return operand;
}

// this function changes value of PC!!!
short Emulator::resolveAddressing(unsigned char offsetCode, unsigned char addressing, int regNum)
{
  unsigned short pc = registers[7];
  short operand = 0;
  switch (addressing)
  {
  case 0: // neposredno
  {

    operand = memory[pc + 3] + (((short)memory[pc + 4]) << 8);
    registers[7] += 5;
    break;
  }
  case 1: // regdir
  {
    operand = registers[regNum];
    registers[7] += 3;
    break;
  }
  case 2: // reg ind
  {
    unsigned short address = this->resolveOperandAdding(offsetCode, regNum);
    operand = memory[address] + (((short)memory[address + 1]) << 8);
    registers[7] += 3;
    break;
  }
  case 3: // reg ind with pom
  {
    unsigned short address = this->resolveOperandAdding(offsetCode, regNum);
    address += (memory[pc + 3] + (((short)memory[pc + 4]) << 8));
    operand = memory[address] + (((short)memory[address + 1]) << 8);
    registers[7] += 5;
    break;
  }
  case 4: // mem
  {
    unsigned short address = memory[pc + 3] + (((short)memory[pc + 4]) << 8);
    operand = memory[address] + (((short)memory[address + 1]) << 8);
    registers[7] += 5;
    break;
  }
  case 5: // reg dir with add
  {
    operand = this->resolveOperandAdding(offsetCode, regNum);
    operand += memory[pc + 3] + (((short)memory[pc + 4]) << 8);
    registers[7] += 5;
    break;
  }
  default:
    break;
  }
  return operand;
}

void Emulator::resolveStoreAddressing(unsigned char offsetCode, unsigned char addressing, int regD, int regS)
{
  int pc = registers[7];
  short operand = 0;
  switch (addressing)
  {
  case 0: // neposredno
  {
    cerr << "ERROR[-13] Emulator error: Illegal addressing operation: STORE instruction IMMED addressing!" << endl;
    exit(-13);
  }
  case 1: // regdir
  {
    registers[regS] = registers[regD];
    registers[7] += 3;
    break;
  }
  case 2: // reg ind
  {
    unsigned short address = this->resolveOperandAdding(offsetCode, regS);
    unsigned char lower = (unsigned char)registers[regD];
    unsigned char upper = (unsigned char)(registers[regD] >> 8);
    memory[address] = lower;
    memory[address + 1] = upper;
    registers[7] += 3;
    break;
  }
  case 3: // reg ind with pom
  {
    unsigned short address = this->resolveOperandAdding(offsetCode, regS);
    address += memory[pc + 3] + (((unsigned short)memory[pc + 4]) << 8);
    unsigned char lower = (unsigned char)registers[regD];
    unsigned char upper = (unsigned char)(registers[regD] >> 8);
    memory[address] = lower;
    memory[address + 1] = upper;
    registers[7] += 5;
    break;
  }
  case 4: // mem
  {
    unsigned short address = memory[pc + 3] + (((unsigned short)memory[pc + 4]) << 8);
    unsigned char lower = (unsigned char)registers[regD];
    unsigned char upper = (unsigned char)(registers[regD] >> 8);
    memory[address] = lower;
    memory[address + 1] = upper;
    registers[7] += 5;
    break;
  }
  case 5: // reg dir with add
  {
    cerr << "ERROR[-13] Emulator error: Illegal addressing operation: STORE instruction"
         << "REEGISTER DIRECT WITH OFFSET addressing!" << endl;
    exit(-13);
  }
  default:
    break;
  }
}

void Emulator::setData()
{
  ifstream inputFile(input_file);
  if (!inputFile.is_open())
  {
    cerr << "ERROR[-10] File error: unable to open file with name " << input_file << "!" << endl;
    exit(-10);
  }
  regex dataReg("Data after linking:");
  string line;
  smatch s;
  while (!inputFile.eof())
  {
    getline(inputFile, line);
    if (!regex_search(line, s, dataReg))
      continue;
    else
    {
      getline(inputFile, line);
      break;
    }
  }
  int counter = 0;
  while (!inputFile.eof() && !line.empty())
  {
    int position = line.find_first_of(":");
    string num = line.substr(position + 1);

    for (int i = 0; i < 8; i++)
    {
      position = num.find_first_not_of(" ");
      if (position == -1)
      {
        inputFile.close();
        return;
      }
      num = num.substr(position);
      position = num.find_first_of(" ");
      string data = num.substr(0, position);
      num = num.substr(position);

      unsigned char c = (unsigned char)stoi(data, nullptr, 16);
      memory[counter++] = c;
    }
    getline(inputFile, line);
  }

  inputFile.close();
}