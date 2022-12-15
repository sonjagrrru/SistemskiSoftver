#include "../inc/Symbol.hpp"
#include <sstream>
#include <iostream>
#include <iomanip>

int Symbol::counter = 0;

Symbol::Symbol(string name, string sectionName, int value, SymbolType type, int offset, string input_file)
{
  this->id = counter++;
  this->name = name;
  this->offset = offset;
  this->sectionName = sectionName;
  this->type = type;
  this->value = value;
  this->input_file = input_file;
}

void Symbol::setSection(Section *s) { this->section = s; } 
void Symbol::setId(int id) { this->id = id; }
string Symbol::getName() { return name; }
string Symbol::getSectionName() { return sectionName; }
SymbolType Symbol::getType() { return type; }
string Symbol::getFileName() { return input_file; }
void Symbol::setValue(short val) { value = val; }
int Symbol::getOffset() { return offset; }
Section *Symbol::getSection() { return section; }
void Symbol::setOffset(int offset) { this->offset = offset; }
int Symbol::getValue() { return value; }

ostream &operator<<(ostream &os, const Symbol &s)
{
  os << left << setw(7) << dec << s.id << '|';
  os << left << setw(25) << s.name << '|';
  os << left << setw(25) << s.sectionName << '|';
  os << left << setw(7) << hex << s.value << '|';
  os << left << setw(7) << (s.type == GLOBAL_SYM ? "GLOBAL" : "LOCAL") << '|';
  os << endl;
  return os;
}