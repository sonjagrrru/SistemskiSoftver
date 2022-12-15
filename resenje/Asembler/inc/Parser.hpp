#ifndef PARSER_HPP
#define PARSER_HPP

#include <regex>
#include <list>
#include <string>

using namespace std;

class Parser
{
public:
  string isDirective(string line);
  bool isNumber(string line);
  string cutBlankspaces(string line);
  list<string> getParameterList(string line);
  int getLiteral(string line);
  int getSkipLiteral(string line);
  string getSectionName(string line);
  bool isLineComment(string line);
  string isLabel(string line);
  string isInstruction(string line);
  bool resolveNoParamInst(string line);
  int resolveOneRegParamInst(string line);
  list<int> resolveTwoRegRaramInst(string line);
  string getAsciiString(string line);
  int getEquLiteral(string line);
  string getEquSymbol(string line);
};

#endif