#ifndef LINKER_HPP
#define LINKER_HPP

#include "../inc/Parser.hpp"
#include "../inc/Section.hpp"
#include "../inc/Symbol.hpp"
#include "../inc/RealocationSymbol.hpp"
#include <string>
#include <list>
#include <fstream>
#include <iostream>

#define STACK_SLOT 1024

using namespace std;

class Linker
{
private:
  int memorySize;
  list<Section *> sections;
  list<Symbol *> symbols;
  list<string> input_files;
  list<RealocationSymbol *> realocs;
  list<unsigned char> data;

  Section *ABS;
  Section *UNDEF;

  ostream *output_file;

public:
  Linker(ostream *output, list<string> input);
  void addSections(list<Section *> list);
  void addSymbols(list<Symbol *> list);
  void addRealocSymbols(list<RealocationSymbol *> list);
  Section *getSection(string name, string input_file);
  Symbol *getSymbol(string name, string input_file);
  void resolveUndefSymbols();
  Symbol *findGlobal(string name);
  void orderSectionsLevelA();
  void checkSymbol(Symbol *s);
  void addData(list<unsigned char> data);
  void resolveSymbols();
  void resolveRealocs();
  void Pass();
  void getSectionData();
  void printInFile();
  void printTables();
  void washSymbols();
  void washSections();
};

#endif