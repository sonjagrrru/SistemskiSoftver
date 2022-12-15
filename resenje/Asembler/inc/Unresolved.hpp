#ifndef UNRESOLVED_HPP
#define UNRESOLVED_HPP

#include <string>
#include "../inc/Section.hpp"

using namespace std;

class Unresolved
{
private:
  string name;
  Section *section;
  int offset;
  
  //+ -> add
  //- -> sub
  //G -> set as global
  //I -> initialize symbol
  char operation;
  int value;

public:
  Unresolved(string name, Section *section, int offset, char operation);
  string getName();
  Section* getSection();
  int getOffset();
  char getOperation();
  void setValue(int value);
};

#endif