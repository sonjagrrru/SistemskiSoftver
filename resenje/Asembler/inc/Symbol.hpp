#ifndef SYMBOL_HPP
#define SYMBOL_HPP

#include <string>
#include "../inc/Section.hpp"

using namespace std;

typedef enum
{
  GLOBAL_SYM = 0,
  LOCAL_SYM
} SymbolType;

class Symbol
{
private:
  static int counter;
  int id;
  string name;
  int value;
  SymbolType type;
  Section *section;
  int offset;

public:
  Symbol(string name, int value, SymbolType type, Section *section);
  string getName();
  long getValue();
  SymbolType getType();
  Section* getSection();
  void setType(SymbolType type);
  void setValue(int value);
  void setOffset(int offset);

  friend ostream& operator <<(ostream &os,const Symbol &s);
};

#endif