#include "../inc/Linker.hpp"
#include <iterator>
#include <sstream>
#include <iostream>
#include <iomanip>

Linker::Linker(ostream *output, list<string> input)
{
  output_file = output;
  input_files = input;
  memorySize = 0;
  sections = list<Section *>();
  symbols = list<Symbol *>();
  realocs = list<RealocationSymbol *>();
  data = list<unsigned char>();
  ABS = new Section(0, "ABS", 0, "");
  UNDEF = new Section(1, "UNDEF", 0, "");
  sections.push_back(ABS);
  sections.push_back(UNDEF);
}

void Linker::addSections(list<Section *> list)
{
  while (!list.empty())
  {
    this->sections.push_back(list.front());
    list.pop_front();
  }
}

Section *Linker::getSection(string name, string input_file)
{
  if (name.compare("ABS") == 0)
    return this->ABS;
  if (name.compare("UNDEF") == 0)
    return this->UNDEF;
  Section *ret;
  for (auto i = sections.begin(); i != sections.end(); ++i)
  {
    ret = *i;
    if (ret->getName().compare(name) == 0 && ret->getFile().compare(input_file) == 0)
      return ret;
  }
  return nullptr;
}

Symbol *Linker::getSymbol(string name, string input_file)
{
  Symbol *ret;
  for (auto i = symbols.begin(); i != symbols.end(); ++i)
  {
    ret = *i;
    if (ret->getName().compare(name) == 0 && ret->getFileName().compare(input_file) == 0)
      return ret;
  }
  return nullptr;
}

void Linker::addSymbols(list<Symbol *> list)
{
  while (!list.empty())
  {
    Symbol *s = list.front();
    checkSymbol(s);
    string sectionName = s->getSectionName();
    string symbolName = s->getName();
    string fileName = s->getFileName();
    Section *symSection = getSection(sectionName, fileName);
    s->setSection(symSection);
    if (sectionName.compare("ABS") == 0)
    {
      Section *section = getSection(symbolName, fileName);
      if (section != nullptr)
      {
        section->setSectionSymbol(s);
      }
    }
    this->symbols.push_back(s);
    list.pop_front();
  }
}

void Linker::checkSymbol(Symbol *s)
{
  Symbol *tmp;
  string fileName = s->getFileName();
  string symName = s->getName();
  string sectionName = s->getSectionName();
  if (sectionName.compare("ABS") != 0)
  {
    for (auto i = symbols.begin(); i != symbols.end(); ++i)
    {
      tmp = *i;

      // ako je u istoj sekciji a dva su lokalna
      /// ili ako imaju dva globalna
      if (tmp->getType() == GLOBAL_SYM && s->getType() == GLOBAL_SYM && tmp->getName().compare(symName) == 0 && tmp->getSectionName().compare("UNDEF") != 0 && sectionName.compare("UNDEF") != 0)
      {
        cerr << "ERROR[-6] Linker error: Two global symbols defined with same name [" << symName << "]!" << endl;
        exit(-6);
      }
      if (tmp->getSectionName().compare(sectionName) == 0 && tmp->getName().compare(symName) == 0 &&
          tmp->getSectionName().compare("UNDEF") != 0 && sectionName.compare("UNDEF") != 0)
      {
        cerr << "ERROR[-6] Linker error: Two symbols with same "
             << "name defined in same section[" << symName << "]!" << endl;
        exit(-6);
      }
    }
  }
}

void Linker::addRealocSymbols(list<RealocationSymbol *> list)
{
  while (!list.empty())
  {
    RealocationSymbol *tmp = list.front();
    string fileName = tmp->getFileName();
    string symName = tmp->getSymbolName();
    Symbol *s = getSymbol(symName, fileName);
    tmp->setSymbol(s);
    this->realocs.push_back(tmp);
    list.pop_front();
  }
}

Symbol *Linker::findGlobal(string name)
{
  Symbol *s;
  for (auto i = symbols.begin(); i != symbols.end(); ++i)
  {
    s = *i;
    if (s->getName().compare(name) == 0 && s->getSectionName().compare("UNDEF") != 0 &&
        s->getType() == GLOBAL_SYM)
    {
      return s;
    }
  }
  return nullptr;
}

void Linker::resolveUndefSymbols()
{
  Symbol *s;
  for (auto i = symbols.begin(); i != symbols.end(); ++i)
  {
    s = *i;
    if (s->getSectionName().compare("UNDEF") == 0)
    {
      Symbol *globalS = findGlobal(s->getName());
      if (globalS == nullptr)
      {
        cerr << "ERROR[-6] Linker error: Unresolved symbol [" << s->getName() << "]!" << endl;
        exit(-6);
      }
      s->setSection(globalS->getSection());
      s->setValue(globalS->getValue());
    }
  }
}

void Linker::orderSectionsLevelA()
{
  list<Section *> tmpSection(sections);
  Section *s;
  // IVT NA PRVO MESTO
  for (auto i = tmpSection.begin(); i != tmpSection.end(); ++i)
  {
    s = *i;
    if (s->getName().compare("ivt") != 0)
      continue;
    if (s->getSectionSize() > 0)
    {
      Symbol *symbol = s->getSectionSymbol();
      symbol->setValue(memorySize);
      memorySize += s->getSectionSize();
      this->addData(s->getData());
      Section *sameName;
      bool firstNeighbour = false;
      for (auto iter = ++i; iter != tmpSection.end(); ++iter)
      {
        sameName = *iter;
        if(iter==i && sameName->getName().compare(s->getName()) == 0)
        {
          --i;
          firstNeighbour=true;
        }
        if (sameName->getName().compare(s->getName()) == 0)
        {
          sameName->getSectionSymbol()->setValue(memorySize);
          memorySize += sameName->getSectionSize();
          this->addData(sameName->getData());
          tmpSection.erase(iter--);
        }
      }
      if(!firstNeighbour)
        --i;
      
    }
    tmpSection.erase(i--);
  }
  // POSLE OSTALE
  for (auto i = tmpSection.begin(); i != tmpSection.end(); ++i)
  {
    s = *i;
    if (s->getName().compare("ivt") == 0)
      continue;
    if (s->getSectionSize() > 0)
    {
      Symbol *symbol = s->getSectionSymbol();
      symbol->setValue(memorySize);
      memorySize += s->getSectionSize();
      this->addData(s->getData());
      Section *sameName;
      bool firstNeighbour = false;
      for (auto iter = ++i; iter != tmpSection.end(); ++iter)
      {
        sameName = *iter;
        if(iter==i && sameName->getName().compare(s->getName()) == 0)
        {
          --i;
          firstNeighbour=true;
        }
        if (sameName->getName().compare(s->getName()) == 0)
        {
          sameName->getSectionSymbol()->setValue(memorySize);
          memorySize += sameName->getSectionSize();
          this->addData(sameName->getData());
          tmpSection.erase(iter--);
        }
      }
      if(!firstNeighbour)
        --i;
    }
    tmpSection.erase(i--);
    
  }
  if (memorySize + STACK_SLOT > 65280)
  {
    cerr << "ERROR[-12] FATAL Memory error: MEMORY OVERFLOW!" << endl;
    exit(-12);
  }
}

void Linker::addData(list<unsigned char> data)
{
  while (!data.empty())
  {
    this->data.push_back(data.front());
    data.pop_front();
  }
}

void Linker::resolveSymbols()
{
  Symbol *s;
  for (auto i = symbols.begin(); i != symbols.end(); ++i)
  {
    s = *i;
    if (s->getSectionName().compare("ABS") != 0 && s->getValue() == 65536 &&
        s->getSectionName().compare("UNDEF") != 0)
    {
      Section *sec = s->getSection();
      int sectionOffset = sec->getSectionSymbol()->getValue();
      s->setValue(s->getOffset() + sectionOffset);
    }
  }
}

void Linker::resolveRealocs()
{
  RealocationSymbol *rs;
  for (auto i = realocs.begin(); i != realocs.end(); ++i)
  {
    rs = *i;
    int value = rs->getSymbol()->getValue();
    unsigned char c1 = (unsigned char)value;
    unsigned char c2 = (unsigned char)(value >> 8);
    Section *realocSection = getSection(rs->getSectionName(), rs->getFileName());
    int address = realocSection->getSectionSymbol()->getValue() + rs->getOffset();
    int counter = 0;
    list<unsigned char> data2 = list<unsigned char>();
    for (auto iter = data.begin(); iter != data.end(); ++iter)
    {
      if (counter == address)
      {
        data2.push_back(c1);
      }
      else if (counter == address + 1)
      {
        data2.push_back(c2);
      }
      else
      {
        data2.push_back(*iter);
      }
      counter++;
    }
    data = data2;
  }
}

void Linker::getSectionData()
{
  Section *s;
  for (auto i = sections.begin(); i != sections.end(); ++i)
  {
    s = *i;
    Parser *newParser = new Parser(s->getFile());
    if (s->getSectionSize() > 0)
    {
      newParser->getData(s);
    }
  }
}

void Linker::printTables()
{
  //--------------print symbols-------------------------
  cout << "\n-----------------------------Tabela simbola --------------------------------\n";
  *output_file << "------------------------------Tabela simbola -------------------------------\n";
  cout << "  ID   |           NAME          |          SECTION        | VALUE |  TYPE |\n";
  *output_file << "  ID   |           NAME           |          SECTION        | VALUE |  TYPE |\n";
  for (auto i = symbols.begin(); i != symbols.end(); ++i)
  {
    Symbol *symb = *i;
    cout << *symb;
    *output_file << *symb;
  }
  cout << "----------------------------------------------------------------------------\n";
  *output_file << "------------------------------------------------------------------------------\n";

  //-------------print sections-------------------------
  cout << "\n------------------Tabela sekcija-----------------\n";
  cout << "    ID    |           NAME          |  SIZE(B) |\n";
  *output_file << "\n------------------Tabela sekcija-----------------\n";
  *output_file << "    ID    |           NAME          |  SIZE(B) |\n";
  for (auto it = sections.begin(); it != sections.end(); ++it)
  {
    Section *sec = *it;
    cout << *sec;
    *output_file << *sec;
  }
  cout << "--------------------------------------\n\n";
  *output_file << "---------------------------------------\n\n";
}

void Linker::washSymbols()
{
  list<Symbol *> tmpSymbol(symbols);
  Symbol *s;
  int counter = 0;
  for (auto i = tmpSymbol.begin(); i != tmpSymbol.end(); ++i)
  {
    s = *i;
    if (s->getSectionName().compare("UNDEF") == 0)
    {
      tmpSymbol.erase(i--);
      continue;
    }
    s->setId(counter++);
    Symbol *sameName;
    bool found = false;
    for (auto iter = ++i; iter != tmpSymbol.end(); ++iter)
    {
      sameName = *iter;
      if (sameName->getName().compare(s->getName()) == 0)
      {
        if(found == false){
          --i;
        }
        found = true;
        tmpSymbol.erase(iter--);
      }
    }
    if(!found)
      --i;
  }
  symbols = tmpSymbol;
}

void Linker::washSections()
{
  list<Section *> tmpSection(sections);
  Section *s;
  int counter = 0;
  for (auto i = tmpSection.begin(); i != tmpSection.end(); ++i)
  {
    s = *i;
    s->setId(counter++);
    Section *sameName;
    bool found = false;
    for (auto iter = ++i; iter != tmpSection.end(); ++iter)
    {
      sameName = *iter;
      if (sameName->getName().compare(s->getName()) == 0)
      {
        s->appendSize(sameName->getSectionSize());
        auto temp = iter;
        if(found == false){
          --i;
        }
        found = true;
        
        --iter;
        tmpSection.erase(temp);
      }
    }

    if(!found)
      --i;
    
  }
  sections = tmpSection;
}

void Linker::printInFile()
{
  this->printTables();
  cout << "Data after linking:" << endl;
  *output_file << "Data after linking:" << endl;
  unsigned char tmpData;
  int newLine = 0;
  for (auto i = data.begin(); i != data.end(); ++i)
  {
    if (newLine % 8 == 0)
    {
      cout << left << setw(4) << hex << static_cast<int>(newLine) << ": ";
      *output_file << left << setw(4) << hex << static_cast<int>(newLine) << ": ";
    }
    tmpData = *i;
    cout << left << setw(3) << hex << static_cast<int>(tmpData) << " ";
    *output_file << left << setw(3) << hex << static_cast<int>(tmpData) << " ";
    if (newLine % 8 == 7)
    {
      cout << endl;
      *output_file << endl;
    }
    newLine++;
  }
  cout << endl;
  *output_file << endl;
}

void Linker::Pass()
{
  string fileName;
  for (auto i = input_files.begin(); i != input_files.end(); ++i)
  {
    fileName = *i;
    Parser *newParser = new Parser(fileName);
    this->addSections(newParser->getSections());
    this->addSymbols(newParser->getSymbols());
    this->addRealocSymbols(newParser->getRelSymbols());
  }
  this->getSectionData();
  //this->printTables();
  this->orderSectionsLevelA();
  this->resolveSymbols();
  this->resolveUndefSymbols();
  this->resolveRealocs();
  this->washSymbols();
  this->washSections();
  this->printInFile();
}