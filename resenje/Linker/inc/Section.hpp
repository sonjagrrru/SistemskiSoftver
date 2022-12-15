#ifndef SECTION_HPP
#define SECTION_HPP

#include <string>
#include <fstream>
#include <iostream>
#include <list>

class Symbol;

using namespace std;

class Section
{
private:
  int id;
  string name;
  int size;
  string input_file;
  Symbol *section_symbol;
  list<unsigned char> data;

public:
  Section(int id, string name, int size, string input_file);
  string getName();
  int getSectionSize();
  string getFile();
  void appendSize(int size);
  void setSectionSymbol(Symbol *s);
  void addData(list<unsigned char> c);
  list<unsigned char> getData();
  Symbol *getSectionSymbol();
  friend ostream &operator<<(ostream &os, const Section &s);
  void setId(int id);
};

#endif