#include "../inc/RealocationSymbol.hpp"
#include <sstream>
#include <iostream>
#include <iomanip>

RealocationSymbol::RealocationSymbol(string name, int offset, char operation)
{
  this->name = name;
  this->offset = offset;
  this->operation = operation;
}

ostream &operator<<(ostream &os, const RealocationSymbol &rs)
{
  os << left << setw(25) << rs.name << '|';
  os << left << setw(10) << dec << rs.offset << '|';
  os << left << setw(9) << rs.operation << '|';
  os << endl;
  return os;
}