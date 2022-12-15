#include "../inc/Parser.hpp"

Parser::Parser(string input)
{
  this->input_file = input;
}

list<Section *> Parser::getSections()
{
  ifstream inputFile(input_file);
  if (!inputFile.is_open())
  {
    cerr << "ERROR[-10] File error: unable to open file with name " << input_file << "!" << endl;
    exit(-10);
  }
  regex sectionTableRegex("(-)+Tabela[ ]sekcija(-)+");
  regex endSectionTable("^Tabela");
  string line;
  smatch s;
  list<Section *> listOfSections = list<Section *>();
  while (!inputFile.eof())
  {
    getline(inputFile, line);
    if (!regex_search(line, s, sectionTableRegex))
      continue;
    else
    {
      getline(inputFile, line);
      getline(inputFile, line);
      break;
    }
  }
  while (!regex_search(line, s, endSectionTable))
  {
    // id
    int position = line.find_first_of(" ");
    ;
    int id = stoi(line.substr(0, position));

    // name
    position = line.find_first_of("|");
    line = line.substr(position + 1);
    position = line.find_first_of(" ");
    string name = line.substr(0, position);

    // size
    position = line.find_first_of("|");
    line = line.substr(position + 1);
    position = line.find_first_of(" ");
    int size = stoi(line.substr(0, position));

    if (name.compare("ABS") != 0 && name.compare("UNDEF") != 0)
    {
      Section *newSection = new Section(id, name, size, input_file);
      listOfSections.push_back(newSection);
    }
    getline(inputFile, line);
  }
  inputFile.close();
  return listOfSections;
}

list<Symbol *> Parser::getSymbols()
{
  ifstream inputFile(input_file);
  regex symbolTableRegex("(-)+Tabela[ ]simbola[ ](-)+");
  regex endSymbolTable("^(\\s)*$");
  string line;
  smatch s;
  list<Symbol *> listOfSymbols = list<Symbol *>();
  while (!inputFile.eof())
  {
    getline(inputFile, line);
    if (!regex_search(line, s, symbolTableRegex))
      continue;
    else
    {
      getline(inputFile, line);
      getline(inputFile, line);
      break;
    }
  }
  while (!regex_search(line, s, endSymbolTable))
  {
    // id
    int position = line.find_first_of(" ");
    int id = stoi(line.substr(0, position));

    // name
    position = line.find_first_of("|");
    line = line.substr(position + 1);
    position = line.find_first_of(" ");
    string name = line.substr(0, position);

    // section name
    position = line.find_first_of("|");
    line = line.substr(position + 1);
    position = line.find_first_of(" ");
    string sectionName = line.substr(0, position);

    // value
    position = line.find_first_of("|");
    line = line.substr(position + 1);
    position = line.find_first_of(" ");
    int value = stoi(line.substr(0, position));

    // type
    position = line.find_first_of("|");
    line = line.substr(position + 1);
    position = line.find_first_of(" ");
    string symbolType = line.substr(0, position);
    SymbolType type = LOCAL_SYM;
    if (symbolType.compare("GLOBAL") == 0)
      type = GLOBAL_SYM;

    // offset
    position = line.find_first_of("|");
    line = line.substr(position + 1);
    position = line.find_first_of(" ");
    int offset = stoi(line.substr(0, position));

    Symbol *newSymbol = new Symbol(name, sectionName, value, type, offset, input_file);
    listOfSymbols.push_back(newSymbol);

    getline(inputFile, line);
  }
  inputFile.close();
  return listOfSymbols;
}

list<RealocationSymbol *> Parser::getRelSymbols()
{
  ifstream inputFile(input_file);
  regex relTableRegex("Tabela[ ]realokacija[ ]");
  regex endRelTable("^(-)+$");
  string line;
  smatch s;
  list<RealocationSymbol *> listOfRealoc = list<RealocationSymbol *>();
  while (!inputFile.eof())
  {
    string sectionName;
    getline(inputFile, line);
    if (!regex_search(line, s, relTableRegex))
      continue;
    else
    {
      sectionName = s.suffix().str();
      int position = sectionName.find_first_of(" ");
      sectionName = sectionName.substr(0, position);
      getline(inputFile, line);
      getline(inputFile, line);
    }

    while (!regex_search(line, s, endRelTable))
    {

      // name
      int position = line.find_first_of(" ");
      string name = line.substr(0, position);

      // offset
      position = line.find_first_of("|");
      line = line.substr(position + 1);
      position = line.find_first_of(" ");
      int offset = stoi(line.substr(0, position));

      // operation
      position = line.find_first_of("|");
      line = line.substr(position + 1);
      position = line.find_first_of(" ");
      string symbolType = line.substr(0, position);
      char operation = line.at(0);

      string fileName = input_file;

      RealocationSymbol *newSymbol = new RealocationSymbol(fileName, sectionName, name, offset, operation);
      listOfRealoc.push_back(newSymbol);
      getline(inputFile, line);
    }
  }
  inputFile.close();
  return listOfRealoc;
}

void Parser::getData(Section *sec)
{
  ifstream inputFile(input_file);
  regex dataTableRegex("(-)+DATA[ ]OF[ ]SECTION(-)+");
  regex endDataTable("^(-)+$");
  string line;
  smatch s;
  list<unsigned char> data = list<unsigned char>();
  while (!inputFile.eof())
  {
    string sectionName;
    getline(inputFile, line);
    if (!regex_search(line, s, dataTableRegex))
      continue;
    else
    {
      getline(inputFile, line);
      sectionName = line;
      getline(inputFile, line);
    }

    if (sectionName.compare(sec->getName()) != 0)
      continue;

    while (!regex_search(line, s, endDataTable))
    {

      for (int i = 0; i < 10; i++)
      {
        int position = line.find_first_of(" ");
        string num = line.substr(0, position);
        if (num.compare("") == 0)
          break;
        line = line.substr(position + 1);
        unsigned char c = (unsigned char)stoi(num, nullptr, 16);
        data.push_back(c);
      }
      getline(inputFile, line);
    }
    sec->addData(data);
    break;
  }
}