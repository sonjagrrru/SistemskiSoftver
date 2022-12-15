#include "../inc/Parser.hpp"
#include <iostream>

string Parser::isDirective(string line)
{
  size_t position = line.find_first_not_of(" ");
  string tmp = line.substr(position);
  regex r("\\.(\\w+)([ ]+(\\w)+(,[ ]*(\\w)+[ ]*)*)?");
  regex ascii("^.ascii");
  smatch s;
  if (regex_search(tmp, s, ascii))
  {
    return line;
  }
  if (regex_search(tmp, s, r))
  {
    return s.str();
  }
  return "nullptr";
}

string Parser::cutBlankspaces(string line)
{
  size_t position = line.find_first_of(" ");
  //cout<<"katujem"<<endl;
  return line.substr(0, position);
}

bool Parser::isNumber(string line)
{
  size_t position = line.find_first_not_of(" ");
  string tmp = line.substr(position);
  regex hex("^0x[0-9a-fA-F]+[ ]*");
  regex dec("^[0-9]+[ ]*");
  smatch s;
  if (regex_search(tmp, s, dec))
  {
    return true;
  }
  if (regex_search(tmp, s, hex))
  {
    return true;
  }
  return false;
}

// blanko znaci iza zareza!!!
list<string> Parser::getParameterList(string line)
{
  size_t position = line.find_first_not_of(" ");
  string tmp = line.substr(position);
  regex r("^\\.(\\w+)[ ]*");
  smatch s;
  list<string> tmpList = list<string>();
  char delimiter = ',';
  if (regex_search(tmp, s, r))
  {
    tmp = s.suffix().str();
    // cout<<tmp<<endl;
    position = tmp.find_first_of(delimiter);
    while (position != -1)
    {
      // cout<<position<<endl;
      string parameter = tmp.substr(0, position);
      int first = parameter.find_first_not_of(" ");
      int last = parameter.find_last_not_of(" ");
      parameter = parameter.substr(first, last + 1);
      tmpList.push_back(parameter);
      tmp = tmp.substr(position + 1);
      position = tmp.find_first_of(delimiter);
      //cout<<parameter<<endl;
    }
    if (tmp != "")
    {
      int first = tmp.find_first_not_of(" ");
      int last = tmp.find_last_not_of(" ");
      tmp = tmp.substr(first, last + 1);
      //cout<<tmp<<endl;
      tmpList.push_back(tmp);
    }
  }
  return tmpList;
}

int Parser::getLiteral(string line)
{
  size_t position = line.find_first_not_of(" ");
  string tmp = line.substr(position);
  regex dec("[0-9]+");
  regex hex("0x[0-9a-fA-F]+");
  smatch s;
  if (regex_search(tmp, s, hex))
  {
    return stoi(s.str(), nullptr, 16);
  }
  if (regex_search(tmp, s, dec))
  {
    return stoi(s.str());
  }
  return -100000;
}

int Parser::getSkipLiteral(string line)
{
  size_t position = line.find_first_not_of(" ");
  string tmp = line.substr(position);
  regex r("^\\.(\\w+)[ ]*");
  regex hex("^0x[0-9a-fA-F]+$");
  regex dec("[0-9]+");
  smatch s;
  if (regex_search(tmp, s, r))
  {
    tmp = s.suffix().str();
    if (regex_search(tmp, s, hex))
    {
      return stoi(s.str(), nullptr, 16);
    }
    if (regex_search(tmp, s, dec))
    {
      return stoi(s.str());
    }
  }

  return 100000;
}

string Parser::getSectionName(string line)
{
  size_t position = line.find_first_not_of(" ");
  string tmp = line.substr(position);
  regex r("^\\.(\\w+)[ ]*");
  smatch s;
  if (regex_search(tmp, s, r))
  {
    tmp = s.suffix().str();
    int first = tmp.find_first_not_of(" ");
    int last = tmp.find_last_not_of(" ");
    // cout<<tmp.substr(first,last+1)<<endl;
    return tmp.substr(first, last + 1);
  }
  return "error";
}

bool Parser::isLineComment(string line)
{
  size_t position = line.find_first_not_of(" ");
  string tmp = line.substr(position);
  regex r("^#[ ]");
  smatch s;
  if (regex_search(tmp, s, r))
  {
    return true;
  }
  return false;
}

string Parser::isLabel(string line)
{
  size_t comPosition = line.find_first_of('#');
  string noComm = line;
  if (comPosition != std::string::npos)
    noComm = line.substr(0, comPosition);
  size_t position = noComm.find_first_not_of(" ");
  string tmp = noComm.substr(position);
  regex r("^[a-zA-Z](\\w+):");
  smatch s;
  if (regex_search(tmp, s, r))
  {
    position = s.str().find_first_of(":");
    return s.str().substr(0, position);
  }
  return "nullptr";
}

string Parser::isInstruction(string line)
{
  size_t comPosition = line.find_first_of('#');
  string noComm = line;
  if (comPosition != std::string::npos)
    noComm = line.substr(0, comPosition);
  regex r("^(\\s)+[a-zA-Z]+"); // samo da se vidi da ima ime i nema ':', ostalo se posle resava
  smatch s;
  if (regex_search(noComm, s, r))
  {
    return noComm;
  }
  return "nullptr";
}

bool Parser::resolveNoParamInst(string line)
{
  regex r("^(\\s)*[a-zA-Z]+(\\s)*$");
  smatch s;
  if (regex_search(line, s, r))
  {
    return true;
  }
  return false;
}

int Parser::resolveOneRegParamInst(string line)
{
  regex r("^(\\s)*[a-zA-Z]+(\\s)+r[0-8](\\s)*$");
  smatch s;
  if (regex_search(line, s, r))
  {
    regex regNum("[0-8]");
    if (regex_search(line, s, regNum))
      // cout << "one param " << stoi(s.str()) << endl;
      return stoi(s.str());
  }
  return -1;
}

list<int> Parser::resolveTwoRegRaramInst(string line)
{
  size_t position = line.find_first_not_of(" ");
  string tmp = line.substr(position);
  regex r("^(\\s)*[a-zA-Z]+(\\s)+r[0-8](\\s)*,(\\s)*r[0-8](\\s)*$");
  smatch s;
  list<int> listOfIndex = list<int>();
  if (regex_search(tmp, s, r))
  {
    tmp = s.str();
    regex regNum("[0-8]");
    if (regex_search(tmp, s, regNum))
    {
      // cout << "first reg is " << stoi(s.str());
      listOfIndex.push_back(stoi(s.str()));
    }
    position = tmp.find_first_of(",");
    tmp = tmp.substr(position);
    if (regex_search(tmp, s, regNum))
    {
      // cout << " and second reg is " << stoi(s.str()) << endl;
      listOfIndex.push_back(stoi(s.str()));
    }
  }
  return listOfIndex;
}

string Parser::getAsciiString(string line)
{
  size_t position = line.find_first_not_of(" ");
  string tmp = line.substr(position);
  regex r("^(\\.)ascii(\\s)+");
  smatch s;
  string ret;
  if (regex_search(tmp, s, r))
  {
    position = line.find_first_of('\'');
    ret = line.substr(position + 1);
    position = ret.find_first_of('\'');
    ret = ret.substr(0, position);
    return ret;
  }
  return "";
}

int Parser::getEquLiteral(string line)
{
  size_t position = line.find_first_not_of(" ");
  string tmp = line.substr(position);
  regex r("^(\\.)equ(\\s)+(\\w)+(\\s)*,(\\s)*");
  smatch s;
  if (regex_search(tmp, s, r))
  {
    tmp = s.suffix().str();
    regex dec("[0-9]+");
    regex hex("0x[0-9a-fA-F]+");
    smatch s;
    if (regex_search(tmp, s, hex))
    {
      return stoi(s.str(), nullptr, 16);
    }
    if (regex_search(tmp, s, dec))
    {
      return stoi(s.str());
    }
  }
  return -100000;
}

string Parser::getEquSymbol(string line)
{
  size_t position = line.find_first_not_of(" ");
  string tmp = line.substr(position);
  regex r("^(\\.)equ(\\s)+");
  smatch s;
  string ret;
  if (regex_search(tmp, s, r))
  {
    ret = s.suffix().str();
    position = ret.find_first_of(',');
    ret = ret.substr(0, position);
    position = ret.find_last_not_of(' ');
    ret = ret.substr(0, position + 1);
    return ret;
  }
  return "";
}