#ifndef REALOCATION_SYMBOL_HPP
#define REALOCATION_SYMBOL_HPP

#include <string>

using namespace std;

class RealocationSymbol
{
private:
  string name;
  int offset;
  char operation;
public:
  RealocationSymbol(string name, int offset, char operation);
  friend ostream& operator <<(ostream &os,const RealocationSymbol &rs);
};

#endif