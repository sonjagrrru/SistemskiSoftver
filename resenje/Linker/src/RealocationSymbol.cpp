#include "../inc/RealocationSymbol.hpp"

RealocationSymbol::RealocationSymbol(string fileName, string sectionName,
                                     string symbolName, int offset, char operation)
{
  this->fileName = fileName;
  this->sectionName = sectionName;
  this->symbolName = symbolName;
  this->offset = offset;
  this->operation = operation;
}
string RealocationSymbol::getFileName() { return fileName; }
string RealocationSymbol::getSectionName() { return sectionName; }
string RealocationSymbol::getSymbolName() { return symbolName; }
void RealocationSymbol::setSymbol(Symbol *s) { this->symbol = s; }
Symbol *RealocationSymbol::getSymbol() { return symbol; }
int RealocationSymbol::getOffset() { return offset; }