#ifndef SECTION_HPP
#define SECTION_HPP

#include <string>
#include <list>
#include "../inc/RealocationSymbol.hpp"

using namespace std;

class Section
{
private:
  static int counter;
  int id;
  string name;
  int sectionSize;
  list<unsigned char> sectionData;
  list<RealocationSymbol *> realocation_table;

public:
  Section(string name = "blank", int size = 0);
  void setData(list<unsigned char> data);
  string getName();
  int getSectionSize();
  list<RealocationSymbol *> getRealocationTable();
  friend ostream &operator<<(ostream &os, const Section &s);
  void printData(ofstream* os);
  void pushRealocSymBack(RealocationSymbol *r);
  void setRealocationTable(list<RealocationSymbol *> table);
  list<unsigned char> getData();
  void swapDataTable(list<unsigned char> table);
};

#endif