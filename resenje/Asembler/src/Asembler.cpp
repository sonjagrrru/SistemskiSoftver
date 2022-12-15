#include "../inc/Asembler.hpp"
#include <iterator>
#include <sstream>
#include <iostream>
#include <iomanip>

Asembler::Asembler(ifstream *is, ofstream *os)
{
  input_file = is;
  output_file = os;
  currentLine = 1;
  locationCounter = 0;
  ABS = new Section("ABS");
  UNDEF = new Section("UNDEF");
  section_table = list<Section *>();
  section_table.push_back(ABS);
  section_table.push_back(UNDEF);
  symbol_table = list<Symbol *>();
  unresolved_symbols = list<Unresolved *>();
}

/*Asembler::~Asembler()
{
  input_file->close();
  output_file->close();
  Symbol *tmpSymbol;
  for (auto i = symbol_table.begin(); i != symbol_table.end(); ++i)
  {
    tmpSymbol = *i;
    symbol_table.pop_front();
    delete (tmpSymbol);
  }
  Section *tmpSection;
  for (auto i = section_table.begin(); i != section_table.end(); ++i)
  {
    tmpSection = *i;
    if (!tmpSection->getRealocationTable().empty())
    {
      RealocationSymbol *tmpRealocSymbol;
      list<RealocationSymbol *> realocationTable = tmpSection->getRealocationTable();
      for (auto i = realocationTable.begin(); i != realocationTable.end(); ++i)
      {
        tmpRealocSymbol = *i;
        realocationTable.pop_front();
        delete (tmpRealocSymbol);
      }
    }
    section_table.pop_front();
    delete (tmpSection);
  }
  Unresolved *tmpUnresolved;
  for (auto i = unresolved_symbols.begin(); i != unresolved_symbols.end(); ++i)
  {
    tmpUnresolved = *i;
    unresolved_symbols.pop_front();
    delete (tmpUnresolved);
  }
}*/

Directive Asembler::getDirective(string line)
{
  string directName = parser.cutBlankspaces(line);

  Directive d;
  if (directName.compare(".global") == 0)
    d = GLOBAL;
  if (directName.compare(".extern") == 0)
    d = EXTERN;
  if (directName.compare(".section") == 0)
    d = SECTION;
  if (directName.compare(".word") == 0)
    d = WORD;
  if (directName.compare(".skip") == 0)
    d = SKIP;
  if (directName.compare(".end") == 0)
    d = END;
  if (directName.compare(".ascii") == 0)
    d = ASCII;
  if (directName.compare(".equ") == 0)
    d = EQU;
  return d;
}

Instruction Asembler::getInstruction(string line)
{
  int position = line.find_first_not_of(" ");
  string inst = line.substr(position);
  inst = parser.cutBlankspaces(inst);
  Instruction i;
  if (inst.compare("halt") == 0)
    i = HALT;
  if (inst.compare("int") == 0)
    i = INT;
  if (inst.compare("iret") == 0)
    i = IRET;
  if (inst.compare("ret") == 0)
    i = RET;
  if (inst.compare("call") == 0)
    i = CALL;
  if (inst.compare("pop") == 0)
    i = POP;
  if (inst.compare("push") == 0)
    i = PUSH;
  if (inst.compare("jmp") == 0)
    i = JMP;
  if (inst.compare("jne") == 0)
    i = JNE;
  if (inst.compare("jeq") == 0)
    i = JEQ;
  if (inst.compare("jgt") == 0)
    i = JGT;
  if (inst.compare("xchg") == 0)
    i = XCHG;
  if (inst.compare("add") == 0)
    i = ADD;
  if (inst.compare("sub") == 0)
    i = SUB;
  if (inst.compare("mul") == 0)
    i = MUL;
  if (inst.compare("div") == 0)
    i = DIV;
  if (inst.compare("cmp") == 0)
    i = CMP;
  if (inst.compare("not") == 0)
    i = NOT;
  if (inst.compare("and") == 0)
    i = AND;
  if (inst.compare("or") == 0)
    i = OR;
  if (inst.compare("xor") == 0)
    i = XOR;
  if (inst.compare("test") == 0)
    i = TEST;
  if (inst.compare("shl") == 0)
    i = SHL;
  if (inst.compare("shr") == 0)
    i = SHR;
  if (inst.compare("ldr") == 0)
    i = LDR;
  if (inst.compare("str") == 0)
    i = STR;

  return i;
}

void Asembler::resolveDirective(string line)
{
  Directive thisDirective = getDirective(line);
  switch (thisDirective)
  {
  case GLOBAL:
  {
    list<string> parameters = parser.getParameterList(line);
    if (parameters.empty())
    {
      cerr << "ERROR[-1] Directive error: Syntax exception near .global at line " << currentLine << endl;
      exit(-1);
    }
    while (!parameters.empty())
    {
      string parameter = parameters.front();
      if (parser.isNumber(parameter))
      {
        cerr << "ERROR[-1] Directive error: Syntax exception near .global at line " << currentLine << endl;
        exit(-1);
      }
      Symbol *currentSymbol = findSymbol(parameter);
      if (!currentSymbol)
      {
        // nepoznat simbol
        unresolved_symbols.push_back(new Unresolved(parameter, currentSection, locationCounter, 'G'));
      }
      else
      {
        currentSymbol->setType(GLOBAL_SYM);
      }
      parameters.pop_front();
    }
    break;
  }
  case EXTERN:
  {
    list<string> parameters = parser.getParameterList(line);
    if (parameters.empty())
    {
      cerr << "ERROR[-1] Directive error: Syntax exception near .extern at line " << currentLine << endl;
      exit(-1);
    }
    while (!parameters.empty())
    {
      string parameter = parameters.front();
      Symbol *currentSymbol = findSymbol(parameter);
      if (currentSymbol)
      {
        cerr << "ERROR[-1] Directive error: Syntax exception near .extern at line " << currentLine << endl;
        cerr << "Symbol already exists!" << endl;
        exit(-1);
      }
      else
      {
        symbol_table.push_back(new Symbol(parameter, 65536, GLOBAL_SYM, UNDEF));
        Unresolved *tmpUnresolved;
        for (auto i = unresolved_symbols.begin(); i != unresolved_symbols.end(); ++i)
        {
          tmpUnresolved = *i;
          if (tmpUnresolved->getName().compare(parameter) == 0)
          {
            Section *realocSection = tmpUnresolved->getSection();
            char realocOperation = tmpUnresolved->getOperation();
            int offset = tmpUnresolved->getOffset();
            if (realocOperation == '+' || realocOperation == '-')
            {
              if (realocSection == currentSection)
                realoc_table.push_back(new RealocationSymbol(parameter, offset, realocOperation));
              else
                realocSection->pushRealocSymBack(
                    new RealocationSymbol(parameter, offset, realocOperation));
              unresolved_symbols.erase(i--);
            }
            else
            {
              cerr << "ERROR[-1] Directive error: Syntax exception near .extern at line " << currentLine << endl;
              cerr << "Illegal extern symbol operation!" << endl;
              exit(-1);
            }
          }
        }
      }
      parameters.pop_front();
    }
    break;
  }
  case SECTION:
  {
    string sectionName = parser.getSectionName(line);
    if (sectionName.compare("error") == 0)
    {
      cerr << "ERROR[-1] Directive error: Syntax exception near .section at line " << currentLine << endl;
      exit(-1);
    }
    Section *oldSection = findSection(sectionName);
    currentSection->setRealocationTable(realoc_table);
    if (oldSection)
    {
      locationCounter = oldSection->getSectionSize();
      currentSection = oldSection;
      realoc_table = oldSection->getRealocationTable();
    }
    else
    {
      Symbol *sectionSymbol = findSymbol(sectionName);
      if (sectionSymbol)
      {
        cerr << "ERROR[-1] Directive error: Syntax exception near .section at line " << currentLine << endl;
        cerr << "Symbol already exists in symbol table!";
        exit(-1);
      }
      Section *newSection = new Section(sectionName);
      sectionSymbol = new Symbol(sectionName, 65536, LOCAL_SYM, ABS);
      symbol_table.push_back(sectionSymbol);
      section_table.push_back(newSection);
      currentSection = newSection;
      realoc_table = newSection->getRealocationTable();
      locationCounter = 0;
    }
    break;
  }
  case WORD:
  {
    list<string> parameters = parser.getParameterList(line);
    if (parameters.empty())
    {
      cerr << "ERROR[-1] Directive error: Syntax exception near .word at line " << currentLine << endl;
      exit(-1);
    }
    while (!parameters.empty())
    {
      string parameter = parameters.front();
      // poravnanje
      if (locationCounter % 2 != 0)
      {
        list<unsigned char> number = {0};
        currentSection->setData(number);
        locationCounter += 1;
      }
      if (parser.isNumber(parameter))
      {
        int value = parser.getLiteral(parameter);
        list<unsigned char> number = list<unsigned char>();
        // little endian
        unsigned char tmp = (unsigned char)value & 0xff;
        number.push_back(tmp);
        tmp = (value >> 8) & 0xff;
        number.push_back(tmp);
        currentSection->setData(number);
        locationCounter += 2;
      }
      else
      {
        Symbol *currentSymbol = findSymbol(parameter);
        if (!currentSymbol)
        {
          // nepoznat simbol
          unresolved_symbols.push_back(new Unresolved(parameter, currentSection, locationCounter, '+'));
          locationCounter += 2;
        }
        else
        {
          int value = currentSymbol->getValue();
          // nije poznata vrednost simbola u vreme asembliranja
          if (value == 65536)
          {
            list<unsigned char> number = {0, 0};
            currentSection->setData(number);
            currentSection->pushRealocSymBack(new RealocationSymbol(parameter, locationCounter, '+'));
            locationCounter += 2;
            realoc_table = currentSection->getRealocationTable();
          }
          else
          {
            list<unsigned char> number = list<unsigned char>();
            unsigned char tmp = (unsigned char)value & 0xff;
            number.push_back(tmp);
            tmp = (unsigned char)(value >> 8) & 0xff;
            number.push_back(tmp);
            currentSection->setData(number);
            locationCounter += 2;
          }
        }
      }
      parameters.pop_front();
    }
    break;
  }
  case SKIP:
  {
    int literal = parser.getSkipLiteral(line);
    if (literal == 100000 || !currentSection)
    {
      cerr << "ERROR[-1] Directive error: Syntax exception near .skip at line " << currentLine << endl;
      exit(-1);
    }
    list<unsigned char> dataList = list<unsigned char>();
    for (int i = 0; i < literal; i++)
    {
      dataList.push_back(0);
    }
    currentSection->setData(dataList);
    locationCounter += literal;
    break;
  }
  case END:
    endOfFile = true;
    break;
  case ASCII:
  {
    string asciiString = parser.getAsciiString(line);
    if (asciiString.compare("") == 0)
    {
      cerr << "ERROR[-1] Directive error: Syntax exception near .ascii at line " << currentLine << endl;
      exit(-1);
    }
    list<unsigned char> dataList = list<unsigned char>();
    for (int i = 0; i < asciiString.length(); i++)
    {
      unsigned char character = (unsigned char)asciiString.at(i);
      dataList.push_back(character);
    }
    currentSection->setData(dataList);
    locationCounter += asciiString.length();
    break;
  }
  case EQU:
  {
    string equName = parser.getEquSymbol(line);
    int value = parser.getEquLiteral(line);
    Symbol *symbolNew = findSymbol(equName);
    if (symbolNew)
    {
      cerr << "ERROR[-3] Label error: Symbol with name " << line << " already exists!" << endl;
      exit(-1);
    }
    if (this->currentSection->getName() == "ABS")
    {
      cerr << "ERROR[-1] Directive error: " + equName + " is defined out of any section! " << endl;
      exit(-1);
    }
    if (value >= 65536)
    {
      cerr << "ERROR[-1] Directive error: ";
      cerr << value;
      cerr << " is out of range! " << endl;
      exit(-1);
    }
    symbolNew = new Symbol(equName, value, LOCAL_SYM, currentSection);
    symbolNew->setOffset(0);
    symbol_table.push_back(symbolNew);
    Unresolved *tmpUnresolved;
    for (auto i = unresolved_symbols.begin(); i != unresolved_symbols.end(); ++i)
    {
      tmpUnresolved = *i;
      if (tmpUnresolved->getName().compare(line) == 0)
      {
        Section *realocSection = tmpUnresolved->getSection();
        char realocOperation = tmpUnresolved->getOperation();
        int offset = tmpUnresolved->getOffset();
        unsigned char c1 = (unsigned char)value;
        unsigned char c2 = (unsigned char)(value >> 8);
        int counter = 0;
        list<unsigned char> data = realocSection->getData();
        if (realocOperation == '+' || realocOperation == '-')
        {
          list<unsigned char> data2 = list<unsigned char>();
          for (auto iter = data.begin(); iter != data.end(); ++iter)
          {
            if (counter == offset)
            {
              data2.push_back(c1);
            }
            else if (counter == offset + 1)
            {
              data2.push_back(c2);
            }
            else
            {
              data2.push_back(*iter);
            }
            counter++;
          }
          realocSection->swapDataTable(data2);
          unresolved_symbols.erase(i--);
        }
        else if (realocOperation == 'G')
        {
          symbolNew->setType(GLOBAL_SYM);
          unresolved_symbols.erase(i--);
        }
        else
        {
          cerr << "ERROR[-1] Directive error: Syntax exception near .extern at line " << currentLine << endl;
          cerr << "Illegal extern symbol operation!" << endl;
          exit(-1);
        }
      }
    }
    break;
  }

  default:
    exit(-1);
    break;
  }
}

void Asembler::resolveLabel(string line)
{
  Symbol *symbolNew = findSymbol(line);
  if (symbolNew)
  {
    /*if (symbolNew.getType() == GLOBAL_SYM ||
        symbolNew.getSection().getName().compare(currentSection.getName()) == 0)
    {*/
    cerr << "ERROR[-3] Label error: Symbol with name " << line << " already exists!" << endl;
    exit(-3);
    //}
  }
  if (this->currentSection->getName() == "ABS")
  {
    std::cerr << "ERROR[-3] Label error: " + line + " is defined out of any section! " << std::endl;
    exit(-3);
  }
  symbolNew = new Symbol(line, 0x10000, LOCAL_SYM, currentSection);
  symbolNew->setOffset(locationCounter);
  symbol_table.push_back(symbolNew);
  Unresolved *tmpUnresolved;
  for (auto i = unresolved_symbols.begin(); i != unresolved_symbols.end(); ++i)
  {
    tmpUnresolved = *i;
    if (tmpUnresolved->getName().compare(line) == 0)
    {
      Section *realocSection = tmpUnresolved->getSection();
      char realocOperation = tmpUnresolved->getOperation();
      int offset = tmpUnresolved->getOffset();
      if (realocOperation == '+' || realocOperation == '-')
      {
        if (realocSection == currentSection)
          realoc_table.push_back(new RealocationSymbol(line, offset, realocOperation));
        else
          realocSection->pushRealocSymBack(
              new RealocationSymbol(line, offset, realocOperation));
        unresolved_symbols.erase(i--);
      }
      else if (realocOperation == 'G')
      {
        symbolNew->setType(GLOBAL_SYM);
        unresolved_symbols.erase(i--);
      }
      else
      {
        cerr << "ERROR[-1] Directive error: Syntax exception near .extern at line " << currentLine << endl;
        cerr << "Illegal extern symbol operation!" << endl;
        exit(-1);
      }
    }
  }
}

void Asembler::resolveInstruction(string line)
{
  Instruction inst = getInstruction(line);
  if (currentSection->getName().compare("ABS") == 0)
  {
    cerr << "ERROR[-4] Instruction error: Instruction must be inside defined section! Error at  "
         << currentLine << " line." << endl;
    exit(-4);
  }
  switch (inst)
  {
  case HALT:
  {
    if (parser.resolveNoParamInst(line))
    {
      list<unsigned char> data = {0x00};
      currentSection->setData(data);
      locationCounter += 1;
    }
    else
    {
      cerr << "ERROR[-4] Instruction error: Invalid format of instruction at  "
           << currentLine << " line." << endl;
      exit(-4);
    }
    break;
  }
  case INT:
  {
    int regNum = parser.resolveOneRegParamInst(line);
    if (regNum != -1)
    {
      list<unsigned char> data = {0x10};
      unsigned char data2 = (((unsigned char)regNum) << 4) & 0xf0;
      data2 |= 0x0f;
      data.push_back(data2);
      currentSection->setData(data);
      locationCounter += 2;
    }
    else
    {
      cerr << "ERROR[-4] Instruction error: Invalid format of instruction at  "
           << currentLine << " line." << endl;
      exit(-4);
    }
    break;
  }
  case IRET:
  {
    if (parser.resolveNoParamInst(line))
    {
      list<unsigned char> data = {0x20};
      currentSection->setData(data);
      locationCounter += 1;
    }
    else
    {
      cerr << "ERROR[-4] Instruction error: Invalid format of instruction at  "
           << currentLine << " line." << endl;
      exit(-4);
    }
    break;
  }
  case RET:
  {
    if (parser.resolveNoParamInst(line))
    {
      list<unsigned char> data = {0x40};
      currentSection->setData(data);
      locationCounter += 1;
    }
    else
    {
      cerr << "ERROR[-4] Instruction error: Invalid format of instruction at  "
           << currentLine << " line." << endl;
      exit(-4);
    }
    break;
  }
  //                    op code   dest stek s-=2 reg.ind.
  // ucitavanje podatka: 1011 0000 dddd 0110 0001 0010
  case PUSH:
  {
    int regNum = parser.resolveOneRegParamInst(line);
    if (regNum != -1)
    {
      list<unsigned char> data = {0xb0};
      unsigned char data2 = (((unsigned char)regNum) << 4) & 0xf0;
      data2 |= 0x06;
      data.push_back(data2);
      data.push_back(0x12);
      currentSection->setData(data);
      locationCounter += 3;
    }
    else
    {
      cerr << "ERROR[-4] Instruction error: Invalid format of instruction at  "
           << currentLine << " line." << endl;
      exit(-4);
    }
    break;
  }
  //                    op code   dest stek s+=2 reg.ind.
  // ucitavanje podatka: 1010 0000 dddd 0110 0001 0100
  case POP:
  {
    int regNum = parser.resolveOneRegParamInst(line);
    if (regNum != -1)
    {
      list<unsigned char> data = {0xa0};
      char data2 = (((char)regNum) << 4) & 0xf0;
      data2 |= 0x06;
      data.push_back(data2);
      data.push_back(0x42);
      currentSection->setData(data);
      locationCounter += 3;
    }
    else
    {
      cerr << "ERROR[-4] Instruction error: Invalid format of instruction at  "
           << currentLine << " line." << endl;
      exit(-4);
    }
    break;
  }
  case NOT:
  {
    int regNum = parser.resolveOneRegParamInst(line);
    if (regNum != -1)
    {
      list<unsigned char> data = {0x80};
      unsigned char data2 = (((unsigned char)regNum) << 4) & 0xf0;
      data.push_back(data2);
      currentSection->setData(data);
      locationCounter += 2;
    }
    else
    {
      cerr << "ERROR[-4] Instruction error: Invalid format of instruction at  "
           << currentLine << " line." << endl;
      exit(-4);
    }
    break;
  }
  case XCHG:
  case ADD:
  case SUB:
  case MUL:
  case DIV:
  case CMP:
  case AND:
  case OR:
  case XOR:
  case TEST:
  case SHL:
  case SHR:
  {
    list<int> regNum = parser.resolveTwoRegRaramInst(line);
    if (regNum.size() == 2)
    {
      list<unsigned char> data;
      if (inst == XCHG)
        data = {0x60};
      if (inst == ADD)
        data = {0x70};
      if (inst == SUB)
        data = {0x71};
      if (inst == MUL)
        data = {0x72};
      if (inst == DIV)
        data = {0x73};
      if (inst == CMP)
        data = {0x74};
      if (inst == AND)
        data = {0x81};
      if (inst == OR)
        data = {0x82};
      if (inst == XOR)
        data = {0x83};
      if (inst == TEST)
        data = {0x84};
      if (inst == SHL)
        data = {0x90};
      if (inst == SHR)
        data = {0x91};
      unsigned char data2 = (((unsigned char)regNum.front()) << 4) & 0xf0;
      data2 |= (unsigned char)regNum.back();
      data.push_back(data2);
      currentSection->setData(data);
      locationCounter += 2;
    }
    else
    {
      cerr << "ERROR[-4] Instruction error: Invalid format of instruction at  "
           << currentLine << " line." << endl;
      exit(-4);
    }
    break;
  }
  case JMP:
  case JEQ:
  case JGT:
  case JNE:
  case CALL:
  {
    list<unsigned char> data = list<unsigned char>();
    if (inst == JMP)
      data.push_back(0x50);
    if (inst == JEQ)
      data.push_back(0x51);
    if (inst == JNE)
      data.push_back(0x52);
    if (inst == JGT)
      data.push_back(0x53);
    if (inst == CALL)
      data.push_back(0x30);
    smatch s;
    regex isLiteral("^(\\s)*[a-zA-Z]+(\\s)+((0x[0-9a-fA-F]+)|[0-9]+)(\\s)*$");
    regex isSymbol("^(\\s)*[a-zA-Z]+(\\s)+[a-zA-Z](\\w)*(\\s)*$");
    regex isPcRelSymbol("^(\\s)*[a-zA-Z]+(\\s)+%[a-zA-Z](\\w)*(\\s)*$");
    regex isMemLiteral("^(\\s)*[a-zA-Z]+(\\s)+(\\*)((0x[0-9a-fA-F]+)|[0-9]+)(\\s)*$");
    regex isMemSymbol("^(\\s)*[a-zA-Z]+(\\s)+(\\*)[a-zA-Z](\\w)*(\\s)*$");
    regex isReg("^(\\s)*[a-zA-Z]+(\\s)+(\\*)r[0-8](\\s)*$");
    regex isMemReg("^(\\s)*[a-zA-Z]+(\\s)+(\\*)(\\[)r[0-8](\\])(\\s)*$");
    regex isRegLit("^(\\s)*[a-zA-Z]+(\\s)+(\\*)(\\[)r[0-8](\\s)*(\\+)(\\s)*((0x[0-9a-fA-F]+)|[0-9]+)(\\s)*(\\])(\\s)*$");
    regex isRegSym("^(\\s)*[a-zA-Z]+(\\s)+(\\*)(\\[)r[0-8](\\s)*(\\+)(\\s)*[a-zA-Z](\\w)*(\\s)*(\\])(\\s)*$");
    if (regex_search(line, s, isLiteral))
    {
      int literal;
      regex getHex("0x[0-9a-fA-F]+");
      regex getDec("[0-9]+");
      if (regex_search(line, s, getHex))
        literal = stoi(s.str(), nullptr, 16);
      else
      {
        regex_search(line, s, getDec);
        literal = stoi(s.str());
      }
      data.push_back(0xf0);
      data.push_back(0x00);
      data.push_back((unsigned char)literal);
      data.push_back((unsigned char)(literal >> 8));
      locationCounter += 5;
    }
    else if (regex_search(line, s, isSymbol))
    {
      regex getSymbol("[a-zA-Z](\\w)*(\\s)*$");
      regex_search(line, s, getSymbol);
      string tmp = s.str();
      int position = tmp.find_last_not_of(" ");
      tmp = tmp.substr(0, position + 1);
      Symbol *currentSymbol = findSymbol(tmp);
      if (!currentSymbol)
      {
        // nepoznat simbol
        unresolved_symbols.push_back(new Unresolved(tmp, currentSection, locationCounter + 3, '+'));
        data.push_back(0xf0);
        data.push_back(0x00);
        data.push_back(0x00);
        data.push_back(0x00);
        locationCounter += 5;
      }
      else
      {
        int value = currentSymbol->getValue();
        // nije poznata vrednost simbola u vreme asembliranja
        if (value == 65536)
        {
          realoc_table.push_back(new RealocationSymbol(tmp, locationCounter + 3, '+'));
          data.push_back(0xf0);
          data.push_back(0x00);
          data.push_back(0x00);
          data.push_back(0x00);
          locationCounter += 5;
        }
        else
        {
          data.push_back(0xf0);
          data.push_back(0x00);
          data.push_back((unsigned char)value);
          data.push_back((unsigned char)(value >> 8));
          locationCounter += 5;
        }
      }
    }
    else if (regex_search(line, s, isPcRelSymbol))
    {
      regex getSymbol("%[a-zA-Z](\\w)*");
      regex_search(line, s, getSymbol);
      string tmp = s.str();
      tmp = tmp.substr(1);
      Symbol *currentSymbol = findSymbol(tmp);
      if (!currentSymbol)
      {
        // nepoznat simbol
        unresolved_symbols.push_back(new Unresolved(tmp, currentSection, locationCounter + 3, '+'));
        data.push_back(0xf7);
        data.push_back(0x00);
        data.push_back(0x00);
        data.push_back(0x00);
        locationCounter += 5;
      }
      else
      {
        int value = currentSymbol->getValue();
        // nije poznata vrednost simbola u vreme asembliranja
        if (value == 65536)
        {
          realoc_table.push_back(new RealocationSymbol(tmp, locationCounter + 3, '+'));
          data.push_back(0xf7);
          data.push_back(0x00);
          data.push_back((unsigned char)(locationCounter + 5)); // samo pomeraj od pocetka sekcije znamo to dodamo
          data.push_back((unsigned char)((locationCounter + 5) >> 8));
          locationCounter += 5;
        }
        else
        {
          data.push_back(0xf7);
          data.push_back(0x00);
          data.push_back((unsigned char)value);
          data.push_back((unsigned char)(value >> 8));
          locationCounter += 5;
        }
      }
    }
    else if (regex_search(line, s, isMemLiteral))
    {
      int literal;
      regex getHex("0x[0-9a-fA-F]+");
      regex getDec("[0-9]+");
      if (regex_search(line, s, getHex))
        literal = stoi(s.str(), nullptr, 16);
      else
      {
        regex_search(line, s, getDec);
        literal = stoi(s.str());
      }
      data.push_back(0xf0);
      data.push_back(0x04);
      data.push_back((unsigned char)literal);
      data.push_back((unsigned char)(literal >> 8));
      locationCounter += 5;
    }
    else if (regex_search(line, s, isReg))
    {
      regex getReg("(\\*)r[0-8]");
      regex_search(line, s, getReg);
      string tmp = s.str();
      regex getNum("[0-8]");
      regex_search(tmp, s, getNum);
      int reg = stoi(s.str());
      data.push_back((0xf0) | ((unsigned char)(reg)));
      data.push_back(0x01);
      locationCounter += 3;
    }
    else if (regex_search(line, s, isMemSymbol))
    {
      regex getSymbol("(\\*)[a-zA-Z](\\w)*");
      regex_search(line, s, getSymbol);
      string tmp = s.str();
      tmp = tmp.substr(1);
      Symbol *currentSymbol = findSymbol(tmp);
      if (!currentSymbol)
      {
        // nepoznat simbol
        unresolved_symbols.push_back(new Unresolved(tmp, currentSection, locationCounter + 3, '+'));
        data.push_back(0xf0);
        data.push_back(0x04);
        data.push_back(0x00);
        data.push_back(0x00);
        locationCounter += 5;
      }
      else
      {
        int value = currentSymbol->getValue();
        // nije poznata vrednost simbola u vreme asembliranja
        if (value == 65536)
        {
          realoc_table.push_back(new RealocationSymbol(tmp, locationCounter + 3, '+'));
          data.push_back(0xf0);
          data.push_back(0x04);
          data.push_back(0x00);
          data.push_back(0x00);
          locationCounter += 5;
        }
        else
        {
          data.push_back(0xf0);
          data.push_back(0x04);
          data.push_back((unsigned char)value);
          data.push_back((unsigned char)(value >> 8));
          locationCounter += 5;
        }
      }
    }
    else if (regex_search(line, s, isMemReg))
    {
      regex getReg("(\\[)r[0-8]");
      regex_search(line, s, getReg);
      string tmp = s.str();
      regex getNum("[0-8]");
      regex_search(tmp, s, getNum);
      int reg = stoi(s.str());
      data.push_back((0xf0) | ((unsigned char)(reg)));
      data.push_back(0x02);
      locationCounter += 3;
    }
    else if (regex_search(line, s, isRegLit))
    {
      int plusPosition = line.find_first_of('+');
      string rear = line.substr(plusPosition);
      int literal;
      regex getHex("0x[0-9a-fA-F]+");
      regex getDec("[0-9]+");
      if (regex_search(rear, s, getHex))
        literal = stoi(s.str(), nullptr, 16);
      else
      {
        regex_search(rear, s, getDec);
        literal = stoi(s.str());
      }
      regex getReg("(\\[)r[0-8]");
      regex_search(line, s, getReg);
      string tmp = s.str();
      regex getNum("[0-8]");
      regex_search(tmp, s, getNum);
      int reg = stoi(s.str());
      data.push_back((0xf0) | ((unsigned char)(reg)));
      data.push_back(0x03);
      data.push_back((unsigned char)literal);
      data.push_back((unsigned char)(literal >> 8));
      locationCounter += 5;
    }
    else if (regex_search(line, s, isRegSym))
    {
      int position = line.find_first_of('+');
      string tmp = line.substr(position);
      regex getSymbol("[a-zA-Z](\\w)*");
      regex_search(tmp, s, getSymbol);
      tmp = s.str();
      Symbol *currentSymbol = findSymbol(tmp);
      regex getReg("(\\[)r[0-8]");
      regex_search(line, s, getReg);
      string tmp2 = s.str();
      regex getNum("[0-8]");
      regex_search(tmp2, s, getNum);
      int reg = stoi(s.str());
      if (!currentSymbol)
      {
        // nepoznat simbol
        unresolved_symbols.push_back(new Unresolved(tmp, currentSection, locationCounter + 3, '+'));
        data.push_back((0xf0) | ((unsigned char)(reg)));
        data.push_back(0x03);
        data.push_back(0x00);
        data.push_back(0x00);
        locationCounter += 5;
      }
      else
      {
        int value = currentSymbol->getValue();
        // nije poznata vrednost simbola u vreme asembliranja
        if (value == 65536)
        {
          realoc_table.push_back(new RealocationSymbol(tmp, locationCounter + 3, '+'));
          data.push_back((0xf0) | ((unsigned char)(reg)));
          data.push_back(0x03);
          data.push_back(0x00);
          data.push_back(0x00);
          locationCounter += 5;
        }
        else
        {
          data.push_back((0xf0) | ((unsigned char)(reg)));
          data.push_back(0x03);
          data.push_back((unsigned char)value);
          data.push_back((unsigned char)(value >> 8));
          locationCounter += 5;
        }
      }
    }
    else
    {
      cerr << "ERROR[-4] Instruction error: Invalid instruction format! Error at  "
           << currentLine << " line." << endl;
      exit(-4);
    }
    currentSection->setData(data);
    break;
  }
  case LDR:
  case STR:
  {
    list<unsigned char> data = list<unsigned char>();
    if (inst == LDR)
      data.push_back(0xa0);
    if (inst == STR)
      data.push_back(0xb0);
    smatch s;
    regex isMemLiteral("^(\\s)*[a-zA-Z]+(\\s)+r[0-8](\\s)*,(\\s)*((0x[0-9a-fA-F]+)|[0-9]+)(\\s)*$");
    regex isMemSymbol("^(\\s)*[a-zA-Z]+(\\s)+r[0-8](\\s)*,(\\s)*[a-zA-Z](\\w)*(\\s)*$");
    regex isPcRelSymbol("^(\\s)*[a-zA-Z]+(\\s)+r[0-8](\\s)*,(\\s)*%[a-zA-Z](\\w)*(\\s)*$");
    regex isLiteral("^(\\s)*[a-zA-Z]+(\\s)+r[0-8](\\s)*,(\\s)*(\\$)((0x[0-9a-fA-F]+)|[0-9]+)(\\s)*$");
    regex isSymbol("^(\\s)*[a-zA-Z]+(\\s)+r[0-8](\\s)*,(\\s)*(\\$)[a-zA-Z](\\w)*(\\s)*$");
    regex isReg("^(\\s)*[a-zA-Z]+(\\s)+r[0-8](\\s)*,(\\s)*r[0-8](\\s)*$");
    regex isMemReg("^(\\s)*[a-zA-Z]+(\\s)+r[0-8](\\s)*,(\\s)*(\\[)r[0-8](\\])(\\s)*$");
    regex isRegLit("^(\\s)*[a-zA-Z]+(\\s)+r[0-8](\\s)*,(\\s)*(\\[)r[0-8](\\s)*(\\+)(\\s)*((0x[0-9a-fA-F]+)|[0-9]+)(\\s)*(\\])(\\s)*$");
    regex isRegSym("^(\\s)*[a-zA-Z]+(\\s)+r[0-8](\\s)*,(\\s)*(\\[)r[0-8](\\s)*(\\+)(\\s)*[a-zA-Z](\\w)*(\\s)*(\\])(\\s)*$");
    if (regex_search(line, s, isMemLiteral))
    {
      size_t position = line.find_first_of(',');
      string front = line.substr(0, position);
      string rear = line.substr(position + 1);
      int literal;
      regex getHex("0x[0-9a-fA-F]+");
      regex getDec("[0-9]+");
      if (regex_search(rear, s, getHex))
        literal = stoi(s.str(), nullptr, 16);
      else
      {
        regex_search(rear, s, getDec);
        literal = stoi(s.str());
      }
      regex firstReg("(\\s)r[0-8]");
      regex_search(front, s, firstReg);
      front = s.str();
      regex getNum("[0-8]");
      regex_search(front, s, getNum);
      int reg = stoi(s.str());
      data.push_back(0xf0 & ((unsigned char)(reg << 4)));
      data.push_back(0x04);
      data.push_back((unsigned char)literal);
      data.push_back((unsigned char)(literal >> 8));
      locationCounter += 5;
    }
    else if (regex_search(line, s, isReg))
    {
      size_t position = line.find_first_of(',');
      string front = line.substr(0, position);
      string rear = line.substr(position + 1);
      regex getReg("r[0-8]");
      regex firstReg("(\\s)r[0-8]");
      regex_search(rear, s, getReg);
      string tmp = s.str();
      regex getNum("[0-8]");
      regex_search(tmp, s, getNum);
      int regSecond = stoi(s.str());
      regex_search(front, s, firstReg);
      tmp = s.str();
      regex_search(tmp, s, getNum);
      int regFirst = stoi(s.str());
      data.push_back((0xf0 & ((unsigned char)(regFirst << 4))) | ((unsigned char)(regSecond)));
      data.push_back(0x01);
      locationCounter += 3;
    }
    else if (regex_search(line, s, isMemSymbol))
    {
      size_t position = line.find_first_of(',');
      string front = line.substr(0, position);
      string rear = line.substr(position + 1);
      regex getSymbol("[a-zA-Z](\\w)*(\\s)*$");
      regex_search(rear, s, getSymbol);
      string tmp = s.str();
      int position1 = tmp.find_last_not_of(" ");
      tmp = tmp.substr(0, position1 + 1);
      regex firstReg("(\\s)r[0-8]");
      regex_search(front, s, firstReg);
      front = s.str();
      regex getNum("[0-8]");
      regex_search(front, s, getNum);
      int reg = stoi(s.str());
      Symbol *currentSymbol = findSymbol(tmp);
      if (!currentSymbol)
      {
        // nepoznat simbol
        unresolved_symbols.push_back(new Unresolved(tmp, currentSection, locationCounter + 3, '+'));
        data.push_back(0xf0 & ((unsigned char)(reg << 4)));
        data.push_back(0x04);
        data.push_back(0x00);
        data.push_back(0x00);
        locationCounter += 5;
      }
      else
      {
        int value = currentSymbol->getValue();
        // nije poznata vrednost simbola u vreme asembliranja
        if (value == 65536)
        {
          realoc_table.push_back(new RealocationSymbol(tmp, locationCounter + 3, '+'));
          data.push_back(0xf0 & ((unsigned char)(reg << 4)));
          data.push_back(0x04);
          data.push_back(0x00);
          data.push_back(0x00);
          locationCounter += 5;
        }
        else
        {
          data.push_back(0xf0 & ((unsigned char)(reg << 4)));
          data.push_back(0x04);
          data.push_back((unsigned char)value);
          data.push_back((unsigned char)(value >> 8));
          locationCounter += 5;
        }
      }
    }
    else if (regex_search(line, s, isPcRelSymbol))
    {
      size_t position = line.find_first_of(',');
      string front = line.substr(0, position);
      string rear = line.substr(position + 1);
      regex getSymbol("%[a-zA-Z](\\w)*");
      regex_search(rear, s, getSymbol);
      string tmp = s.str();
      tmp = tmp.substr(1);
      regex firstReg("(\\s)r[0-8]");
      regex_search(front, s, firstReg);
      front = s.str();
      regex getNum("[0-8]");
      regex_search(front, s, getNum);
      int reg = stoi(s.str());
      Symbol *currentSymbol = findSymbol(tmp);
      if (!currentSymbol)
      {
        // nepoznat simbol
        unresolved_symbols.push_back(new Unresolved(tmp, currentSection, locationCounter + 3, '+'));
        data.push_back(0x07 | ((unsigned char)(reg << 4)));
        data.push_back(0x03);
        data.push_back(0x00);
        data.push_back(0x00);
        locationCounter += 5;
      }
      else
      {
        int value = currentSymbol->getValue();
        // nije poznata vrednost simbola u vreme asembliranja
        if (value == 65536)
        {
          realoc_table.push_back(new RealocationSymbol(tmp, locationCounter + 3, '+'));
          data.push_back(0x07 | ((unsigned char)(reg << 4)));
          data.push_back(0x03);
          data.push_back(0x00); // samo pomeraj od pocetka sekcije znamo to dodamo
          data.push_back(0x00);
          locationCounter += 5;
        }
        else
        {
          data.push_back(0x07 | ((unsigned char)(reg << 4)));
          data.push_back(0x03);
          data.push_back((unsigned char)value);
          data.push_back((unsigned char)(value >> 8));
          locationCounter += 5;
        }
      }
    }
    else if (regex_search(line, s, isLiteral))
    {
      size_t position = line.find_first_of(',');
      string front = line.substr(0, position);
      string rear = line.substr(position + 1);
      int literal;
      regex getHex("0x[0-9a-fA-F]+");
      regex getDec("[0-9]+");
      if (regex_search(rear, s, getHex))
        literal = stoi(s.str(), nullptr, 16);
      else
      {
        regex_search(rear, s, getDec);
        literal = stoi(s.str());
      }
      regex firstReg("(\\s)r[0-8]");
      regex_search(front, s, firstReg);
      front = s.str();
      regex getNum("[0-8]");
      regex_search(front, s, getNum);
      int reg = stoi(s.str());
      data.push_back(0xf0 & ((unsigned char)(reg << 4)));
      data.push_back(0x00);
      data.push_back((unsigned char)literal);
      data.push_back((unsigned char)(literal >> 8));
      locationCounter += 5;
    }
    else if (regex_search(line, s, isSymbol))
    {
      size_t position = line.find_first_of(',');
      string front = line.substr(0, position);
      string rear = line.substr(position + 1);
      regex getSymbol("(\\$)[a-zA-Z](\\w)*");
      regex_search(rear, s, getSymbol);
      string tmp = s.str();
      tmp = tmp.substr(1);
      Symbol *currentSymbol = findSymbol(tmp);
      regex firstReg("(\\s)r[0-8]");
      regex_search(front, s, firstReg);
      front = s.str();
      regex getNum("[0-8]");
      regex_search(front, s, getNum);
      int reg = stoi(s.str());
      if (!currentSymbol)
      {
        // nepoznat simbol
        unresolved_symbols.push_back(new Unresolved(tmp, currentSection, locationCounter + 3, '+'));
        data.push_back(0xf0 & ((unsigned char)(reg << 4)));
        data.push_back(0x00);
        data.push_back(0x00);
        data.push_back(0x00);
        locationCounter += 5;
      }
      else
      {
        int value = currentSymbol->getValue();
        // nije poznata vrednost simbola u vreme asembliranja
        if (value == 65536)
        {
          realoc_table.push_back(new RealocationSymbol(tmp, locationCounter + 3, '+'));
          data.push_back(0xf0 & ((unsigned char)(reg << 4)));
          data.push_back(0x00);
          data.push_back(0x00);
          data.push_back(0x00);
          locationCounter += 5;
        }
        else
        {
          data.push_back(0xf0 & ((unsigned char)(reg << 4)));
          data.push_back(0x00);
          data.push_back((unsigned char)value);
          data.push_back((unsigned char)(value >> 8));
          locationCounter += 5;
        }
      }
    }
    else if (regex_search(line, s, isMemReg))
    {
      size_t position = line.find_first_of(',');
      string front = line.substr(0, position);
      string rear = line.substr(position + 1);
      regex getReg("r[0-8]");
      regex firstReg("(\\s)r[0-8]");
      regex_search(rear, s, getReg);
      string tmp = s.str();
      regex getNum("[0-8]");
      regex_search(tmp, s, getNum);
      int regSecond = stoi(s.str());
      regex_search(front, s, firstReg);
      tmp = s.str();
      regex_search(tmp, s, getNum);
      int regFirst = stoi(s.str());
      data.push_back((0xf0 & ((unsigned char)(regFirst << 4))) | ((unsigned char)(regSecond)));
      data.push_back(0x02);
      locationCounter += 3;
    }
    else if (regex_search(line, s, isRegLit))
    {
      size_t position = line.find_first_of(',');
      string front = line.substr(0, position);
      string rear = line.substr(position + 1);
      int plusPosition = line.find_first_of('+');
      string afterPlus = line.substr(plusPosition);
      int literal;
      regex getHex("0x[0-9a-fA-F]+");
      regex getDec("[0-9]+");
      if (regex_search(afterPlus, s, getHex))
        literal = stoi(s.str(), nullptr, 16);
      else
      {
        regex_search(afterPlus, s, getDec);
        literal = stoi(s.str());
      }
      regex getReg("r[0-8]");
      regex firstReg("(\\s)r[0-8]");
      regex_search(rear, s, getReg);
      string tmp = s.str();
      regex getNum("[0-8]");
      regex_search(tmp, s, getNum);
      int regSecond = stoi(s.str());
      regex_search(front, s, firstReg);
      tmp = s.str();
      regex_search(tmp, s, getNum);
      int regFirst = stoi(s.str());
      data.push_back((0xf0 & ((unsigned char)(regFirst << 4))) | ((unsigned char)(regSecond)));
      data.push_back(0x03);
      data.push_back((unsigned char)literal);
      data.push_back((unsigned char)(literal >> 8));
      locationCounter += 5;
    }
    else if (regex_search(line, s, isRegSym))
    {
      size_t position = line.find_first_of(',');
      string front = line.substr(0, position);
      string rear = line.substr(position + 1);
      int plusPos = line.find_first_of('+');
      string symName = line.substr(plusPos + 1);
      regex getSymbol("[a-zA-Z](\\w)*");
      regex_search(symName, s, getSymbol);
      symName = s.str();
      Symbol *currentSymbol = findSymbol(symName);
      regex getReg("r[0-8]");
      regex firstReg("(\\s)r[0-8]");
      regex_search(rear, s, getReg);
      string tmp = s.str();
      regex getNum("[0-8]");
      regex_search(tmp, s, getNum);
      int regSecond = stoi(s.str());
      regex_search(front, s, firstReg);
      tmp = s.str();
      regex_search(tmp, s, getNum);
      int regFirst = stoi(s.str());
      if (!currentSymbol)
      {
        // nepoznat simbol
        unresolved_symbols.push_back(new Unresolved(symName, currentSection, locationCounter + 3, '+'));
        data.push_back((0xf0 & ((unsigned char)(regFirst << 4))) | ((unsigned char)(regSecond)));
        data.push_back(0x03);
        data.push_back(0x00);
        data.push_back(0x00);
        locationCounter += 5;
      }
      else
      {
        int value = currentSymbol->getValue();
        // nije poznata vrednost simbola u vreme asembliranja
        if (value == 65536)
        {
          realoc_table.push_back(new RealocationSymbol(symName, locationCounter + 3, '+'));
          data.push_back((0xf0 & ((unsigned char)(regFirst << 4))) | ((unsigned char)(regSecond)));
          data.push_back(0x03);
          data.push_back(0x00);
          data.push_back(0x00);
          locationCounter += 5;
        }
        else
        {
          data.push_back((0xf0 & ((unsigned char)(regFirst << 4))) | ((unsigned char)(regSecond)));
          data.push_back(0x03);
          data.push_back((unsigned char)value);
          data.push_back((unsigned char)(value >> 8));
          locationCounter += 5;
        }
      }
    }
    else
    {
      cerr << "ERROR[-4] Instruction error: Invalid instruction format! Error at  "
           << currentLine << " line." << endl;
      exit(-4);
    }
    currentSection->setData(data);
    break;
  }
  default:
    break;
  }
}

Symbol *Asembler::findSymbol(string name)
{
  Symbol *finded = nullptr;
  for (auto i = symbol_table.begin(); i != symbol_table.end(); ++i)
  {
    finded = *i;
    if (finded->getName().compare(name) == 0)
      return finded;
  }
  return nullptr;
}

Section *Asembler::findSection(string name)
{
  Section *finded = nullptr;
  for (auto i = section_table.begin(); i != section_table.end(); ++i)
  {
    finded = *i;
    if (finded->getName().compare(name) == 0)
      return finded;
  }
  return nullptr;
}

void Asembler::Pass()
{
  string line;
  this->locationCounter = 0;
  this->currentSection = ABS;
  this->currentLine = 0;
  this->endOfFile = false;

  while (!input_file->eof() && !endOfFile)
  {
    this->currentLine++;
    getline(*input_file, line);

    regex blanks("^(\\s)+$");
    smatch s;
    if (regex_search(line, s, blanks))
      continue;

    if (line.empty())
      continue;

    if (parser.isLineComment(line))
    {
      continue;
    }
    string isLabel = parser.isLabel(line); // Jako bitan redosled!!! Prvo proveri labelu!
    if (isLabel != "nullptr")
    {
      resolveLabel(isLabel);
      continue;
    }

    string isDirective = parser.isDirective(line); // Check if directive
    if (isDirective != "nullptr")
    {
      resolveDirective(isDirective);
      continue;
    }

    string instruction = parser.isInstruction(line); // Check if instruction;
    if (instruction != "nullptr")
    {
      resolveInstruction(instruction);
      continue;
    }
  }
  if (currentSection->getName().compare("ABS") != 0)
    currentSection->setRealocationTable(realoc_table);
  this->unresolvedAfterPass();
  this->print_sym_table();
  this->print_section_table();
  for (auto it = section_table.begin(); it != section_table.end(); ++it)
  {
    Section *symb = *it;
    printRealocTable(symb);
  }
  this->printData();
  this->endOfFile = false;
}

void Asembler::print_sym_table()
{
  cout << "\n------------------------------Tabela simbola ----------------------------------\n";
  *output_file << "---------------------------------Tabela simbola ----------------------------------\n";
  cout << "  ID   |           NAME          |          SECTION        | VALUE |  TYPE |OFFSET |\n";
  *output_file << "  ID   |           NAME          |          SECTION        | VALUE |  TYPE |OFFSET |\n";

  for (auto it = symbol_table.begin(); it != symbol_table.end(); ++it)
  {
    Symbol *symb = *it;
    cout << *symb;
    *output_file << *symb;
  }
}

void Asembler::print_section_table()
{
  cout << "\n-------------------Tabela sekcija------------------\n";
  cout << "    ID    |           NAME          |  SIZE(B) |\n";
  *output_file << "\n-------------------Tabela sekcija------------------\n";
  *output_file << "    ID    |           NAME          |  SIZE(B) |\n";
  for (auto it = section_table.begin(); it != section_table.end(); ++it)
  {
    Section *symb = *it;
    cout << *symb;
    *output_file << *symb;
  }
}

void Asembler::unresolvedAfterPass()
{
  if (!unresolved_symbols.empty())
  {
    cerr << "ERROR[-2] Unresolved symbol error: Undeclared symbol " << unresolved_symbols.front()->getName() << endl;
    exit(-1);
  }
}

void Asembler::printData()
{
  Section *tmpSection;
  for (auto i = section_table.begin(); i != section_table.end(); ++i)
  {
    tmpSection = *i;
    tmpSection->printData(output_file);
  }
}

void Asembler::printRealocTable(Section *s)
{
  cout << "\nTabela realokacija " << left << setw(25) << s->getName() << "\n";
  *output_file << "Tabela realokacija " << left << setw(25) << s->getName() << "\n";
  cout << "          NAME           |  OFFSET  |OPERATION|\n";
  *output_file << "          NAME           |  OFFSET  |OPERATION|\n";
  list<RealocationSymbol *> realocTable = s->getRealocationTable();
  for (auto itR = realocTable.begin(); itR != realocTable.end(); ++itR)
  {
    RealocationSymbol *realocsymb = *itR;
    cout << *realocsymb;
    *output_file << *realocsymb;
  }
  cout << "----------------------------------------------\n";
  *output_file << "----------------------------------------------\n";
}