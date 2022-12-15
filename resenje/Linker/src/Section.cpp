#include "../inc/Section.hpp"
#include <sstream>
#include <iostream>
#include <iomanip>

Section::Section(int id, string name, int size, string input_file)
{
  this->id = id;
  this->input_file = input_file;
  this->name = name;
  this->size = size;
}

void Section::setSectionSymbol(Symbol *s) { this->section_symbol = s; }
string Section::getName() { return this->name; }
void Section::setId(int id) { this->id = id; }
int Section::getSectionSize() { return this->size; }
string Section::getFile() { return this->input_file; }
void Section::addData(list<unsigned char> data)
{
  while (!data.empty())
  {
    this->data.push_back(data.front());
    data.pop_front();
  }
}
list<unsigned char> Section::getData() { return this->data; }
Symbol *Section::getSectionSymbol() { return this->section_symbol; }
void Section::appendSize(int s) { this->size += s; }
// void Section::setRealocationTable(list<RealocationSymbol *> table) { this->realocation_table = table; }

ostream &operator<<(ostream &os, const Section &s)
{
  os << left << setw(10) << s.id << '|';
  os << left << setw(25) << s.name << '|';
  os << left << setw(10) << hex << s.size << '|';
  os << endl;
  return os;
}