#ifndef PARSER_HPP
#define PARSER_HPP

#include <fstream>
#include <iostream>
#include <string>
#include <list>
#include <regex>
#include "../inc/Section.hpp"
#include "../inc/Symbol.hpp"
#include "../inc/RealocationSymbol.hpp"

using namespace std;

class Parser
{
private:
  string input_file;

public:
  Parser(string input);
  list<Section *> getSections();
  list<Symbol *> getSymbols();
  list<RealocationSymbol *> getRelSymbols();
  void getData(Section *s);
};

#endif