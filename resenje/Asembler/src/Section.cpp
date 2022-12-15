#include "../inc/Section.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

int Section::counter = 0;

Section::Section(string name, int size)
{
  this->id = counter++;
  this->name = name;
  this->sectionSize = size;
  this->realocation_table = list<RealocationSymbol *>();
}

void Section::setData(list<unsigned char> data)
{
  this->sectionSize += data.size();
  while (!data.empty())
  {
    this->sectionData.push_back(data.front());
    data.pop_front();
  }
}

void Section::swapDataTable(list<unsigned char> table)
{
  this->sectionSize = table.size();
  this->sectionData = table;
}

list<RealocationSymbol *> Section::getRealocationTable() { return this->realocation_table; }
string Section::getName() { return this->name; }
int Section::getSectionSize() { return this->sectionSize; }
void Section::setRealocationTable(list<RealocationSymbol *> table) { this->realocation_table = table; }
list<unsigned char> Section::getData() { return sectionData; }

ostream &operator<<(ostream &os, const Section &s)
{
  os << left << setw(10) << s.id << '|';
  os << left << setw(25) << s.name << '|';
  os << left << setw(10) << dec << s.sectionSize << '|';
  os << endl;
  return os;
}

void Section::printData(ofstream *os)
{
  unsigned char tmpData;
  int newLine = 0;
  cout << "\n---------DATA OF SECTION-----------" << endl;
  cout << name << endl;
  *os << "\n---------DATA OF SECTION-----------" << endl;
  *os << name << endl;
  for (auto i = sectionData.begin(); i != sectionData.end(); ++i)
  {
    tmpData = *i;
    cout << hex << static_cast<int>(tmpData) << " ";
    *os << hex << static_cast<int>(tmpData) << " ";
    if (newLine % 10 == 9)
    {
      cout << endl;
      *os << endl;
    }
    newLine++;
  }
  if (sectionSize % 10 != 0)
  {
    cout << "\n";
    *os << "\n";
  }
  cout << "--------------------------------------------\n";
  cout << "TOTAL SECTION SIZE IS " << dec << sectionSize << "." << endl;
  *os << "--------------------------------------------\n";
  *os << "TOTAL SECTION SIZE IS " << dec << sectionSize << "." << endl;
}

void Section::pushRealocSymBack(RealocationSymbol *r) { realocation_table.push_back(r); }