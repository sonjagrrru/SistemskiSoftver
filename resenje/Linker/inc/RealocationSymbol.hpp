#ifndef REALOCATION_SYMBOL_HPP
#define REALOCATION_SYMBOL_HPP

#include <string>
#include "../inc/Symbol.hpp"

using namespace std;

class RealocationSymbol
{
private:
  string fileName;
  int offset;
  char operation;
  string symbolName;
  string sectionName;
  Symbol *symbol;

public:
  RealocationSymbol(string fileName, string sectionName, string symbolName, int offset, char operation);
  string getFileName();
  string getSectionName();
  string getSymbolName();
  void setSymbol(Symbol *s);
  Symbol* getSymbol();
  int getOffset();
};

#endif