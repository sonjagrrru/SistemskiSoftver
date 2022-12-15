#ifndef SYMBOL_HPP
#define SYMBOL_HPP

#include <string>
//#include "../inc/Section.hpp"

class Section;

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
  Section *section;
  int value;
  SymbolType type;
  int offset;
  string input_file;

  string sectionName;

public:
  Symbol(string name, string sectionName, int value, SymbolType type, int offset, string input_file);
  void setSection(Section *s);
  void setId(int id);
  string getName();
  string getSectionName();
  SymbolType getType();
  string getFileName();
  int getOffset();
  Section *getSection();
  void setOffset(int offset);
  void setValue(short val);
  int getValue();

  friend ostream &operator<<(ostream &os, const Symbol &s);
};

#endif