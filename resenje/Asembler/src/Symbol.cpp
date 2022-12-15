#include "../inc/Symbol.hpp"
#include <sstream>
#include <iostream>
#include <iomanip>

int Symbol::counter = 0;

string Symbol::getName() { return name; }
long Symbol::getValue() { return value; }
SymbolType Symbol::getType() { return type; }
Section *Symbol::getSection() { return section; }

Symbol::Symbol(string name, int value, SymbolType type, Section *section)
{
  this->name = name;
  this->value = value;
  this->type = type;
  this->section = section;
  this->id = counter++;
  this->offset = 0;
}

void Symbol::setType(SymbolType type) { this->type = type; }
void Symbol::setOffset(int offset) { this->offset = offset; }
void Symbol::setValue(int value) { this->value = value; }

ostream &operator<<(ostream &os, const Symbol &s)
{
  os << left << setw(7) << dec << s.id << '|';
  os << left << setw(25) << s.name << '|';
  os << left << setw(25) << s.section->getName() << '|';
  os << left << setw(7) << dec << s.value << '|';
  os << left << setw(7) << (s.type == GLOBAL_SYM ? "GLOBAL" : "LOCAL") << '|';
  os << left << setw(7) << dec << s.offset << '|';
  os << endl;
  return os;
}