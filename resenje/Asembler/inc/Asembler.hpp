#ifndef ASEMBLER_HPP
#define ASEMBLER_HPP

#include "../inc/RealocationSymbol.hpp"
#include "../inc/Symbol.hpp"
#include "../inc/Section.hpp"
#include "../inc/Unresolved.hpp"
#include "../inc/Parser.hpp"
#include <list>
#include <fstream>
#include <iostream>

using namespace std;

typedef enum
{
  GLOBAL = 0,
  EXTERN,
  SECTION,
  WORD,
  SKIP,
  END,
  ASCII,
  EQU
} Directive;

typedef enum
{
  HALT = 0,
  INT,
  IRET,
  CALL,
  RET,
  JMP,
  JEQ,
  JNE,
  JGT,
  PUSH,
  POP,
  XCHG,
  ADD,
  SUB,
  MUL,
  DIV,
  CMP,
  NOT,
  AND,
  OR,
  XOR,
  TEST,
  SHL,
  SHR,
  LDR,
  STR
} Instruction;

class Asembler
{
private:
  list<Symbol *> symbol_table;
  list<RealocationSymbol *> realoc_table;
  list<Section *> section_table;
  list<Unresolved *> unresolved_symbols;
  Parser parser;
  ifstream *input_file;
  ofstream *output_file;

  // -1 - directiveError
  int errorFlag;

  // da li treba da zavrsimo asembliranje
  bool endOfFile;

  int locationCounter;
  Section *currentSection;
  //za sekcije
  Section *ABS;
  //za simbole cija je vrednost definisana u drugom .o fajlu
  Section *UNDEF;
  int currentLine;

  Directive getDirective(string line);
  Instruction getInstruction(string line);
  void resolveDirective(string line);
  Symbol* findSymbol(string name);
  Section* findSection(string name);
  void print_sym_table();
  void print_section_table();
  void unresolvedAfterPass();
  void resolveLabel(string line);
  void resolveInstruction(string line);
  void printData();
  void printRealocTable(Section *s);

public:
  Asembler(ifstream *is, ofstream *os);
  //~Asembler();
  void Pass();
};

#endif