#include "../inc/Unresolved.hpp"

Unresolved::Unresolved(string name, Section *section, int offset, char operation)
{
  this->name = name;
  this->offset = offset;
  this->operation = operation;
  this->section = section;
}

string Unresolved::getName() { return this->name; }
Section *Unresolved::getSection() { return this->section; }
int Unresolved::getOffset() { return this->offset; }
char Unresolved::getOperation() { return this->operation; }
void Unresolved::setValue(int value) { this->value = value; }