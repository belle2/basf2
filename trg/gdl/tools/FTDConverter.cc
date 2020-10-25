//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Trigger Utility program
//-----------------------------------------------------------------------------
// Filename : FTDConverter.cc
// Section  : TRG GDL
// Owner    : Yoshi Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : To generate FTD algorithm file from verilog source
//-----------------------------------------------------------------------------
// Version |    Date     | Explanation of changes
//   00.00 | 16-Jan-2014 | Creation
//-----------------------------------------------------------------------------

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <string.h>
#include "trg/trg/Utilities.h"

unsigned DebugLevel = 1;

using namespace std;

vector<string> inputs;
vector<unsigned> inputSizes;
vector<string> inputBits;
vector<string> inputNames;
vector<string> outputs;
map<string, string> logics;
vector<string> expanded;
vector<string> expanded2;
vector<string> expandedcc;

void shrink(char b[800]);
void shrink(string&);
void cosmetic(string&);
void tocc(string&);
void chopWord(char b[800], char c[800]);
void chopWord(string&, string&);
void removeComments(char b[800]);
void getInput(char b[800]);
void getOutput(char b[800]);
void getLogic(char b[800]);
void getLogicGND(string&);
void tofunc(string&);
string breakup(const string& logic);
string decodeLogic(const string& logic);
void kumacOutput(ofstream&, string logic, const string& ftd);

class Term {
public:
  explicit Term(const string&);
  virtual ~Term();

public:
//  unsigned size(void) const;
  string sentence(void) const;
  void expand(void);
  string expanded(void);

private:
  string _sentence;
  vector<string> _words;
  vector<Term> _terms;
  string _expanded;
};

int
main(int argc, char* argv[])
{
  cout << "ftd: version 1.01 : 2007/09/21 Y.Iwasaki" << endl;

  //...Check arguments...
  if (argc != 2) {
    cout << "usage : ftd.v" << endl;
    cout << "    ftd.v : ftd logic file" << endl;

    return -1;
  }

  //...Open file...
  cout << "ftd ... reading verilog file : "
       << argv[1] << endl;
  ifstream file(argv[1], ios::in);
  if (! file.is_open()) {
    cout << "    !!! can not open file : " << argv[1] << endl;
    exit(-1);
  }

  //...Read source...
  char b[800];
  unsigned state = 0;
  while (! file.eof()) {
    file.getline(b, 800);

    //...Remove unnecessary spaces...
    shrink(b);

    if (DebugLevel) {
      string tmp = b;
      shrink(tmp);
      bool tmpWrong = false;
      for (unsigned i = 0; i < tmp.size(); i++) {
        if (b[i] != tmp[i])
          tmpWrong = true;
      }
      cout << "state=" << state << ",b0:[" << b << "], tmpWrong="
           << tmpWrong << endl;
    }

    //...Skip blank line...
    if (b[0] == 0) continue;

    //...Input definitions...
    if (state == 0) {
      if (strstr(b, "Inputs"))
        state = 1;
    } else if (state == 1) {
      getInput(b);
      if (strstr(b, "Outputs"))
        state = 2;
    } else if (state == 2) {
      getOutput(b);
      if (strstr(b, "FTD logics"))
        state = 3;
    } else if (state == 3) {
      string tmp = b;
      if (tmp.find("end") != string::npos)
        break;
      getLogic(b);
    }
  }
  file.close();

  //...Logic decoding...
  for (unsigned i = 0; i < outputs.size(); i++) {
    string logic = decodeLogic(logics[outputs[i]]);
    shrink(logic);
    expanded.push_back(logic);
    string logic_cc = logic;

    //...For algorithm file...
    for (unsigned j = 0; j < inputBits.size(); j++) {
      while (1) {
        char crep[10];
        sprintf(crep, "%d", (int)j);
        string inp = inputBits[j];
        string rep = crep;
        string::size_type p = logic.find(inp);
        if (p != string::npos)
          logic.replace(p, inp.size(), rep, 0, rep.size());
        else
          break;
      }
    }
    shrink(logic);
    cosmetic(logic);
    expanded2.push_back(logic);

    //...For cc file...
    for (unsigned j = 0; j < inputBits.size(); j++) {
      while (1) {
        char crep[10];
        sprintf(crep, "i[%d]", (int)j);
        string inp = inputBits[j];
        string rep = crep;
        string::size_type p = logic_cc.find(inp);
        if (p != string::npos)
          logic_cc.replace(p, inp.size(), rep, 0, rep.size());
        else
          break;
      }
    }

    shrink(logic_cc);
    cosmetic(logic_cc);
    tocc(logic_cc);
    string right_cc = "b[" + Belle2::TRGUtilities::itostring(i) + "] = (";
    logic_cc = right_cc + logic_cc + ");";

    expandedcc.push_back(logic_cc);

    if (DebugLevel) {
      cout << "->" << logic << "<-" << endl;
      cout << "->" << logic_cc << "<-" << endl;

    }
  }

  //...Otain file name...
  string filename = argv[1];
  string::size_type s = filename.find_last_of("/");
  if (s != string::npos)
    filename = filename.substr(s + 1);
  s = filename.find_last_of(".");
  if (s != string::npos)
    filename = filename.substr(0, s);

  //...Summary for text file...
  string tFilename = filename + ".txt";
  string iFilename = filename + ".inp";
  string oFilename = filename + ".oup";
  cout << "    Text file  :" << tFilename << endl;
  ofstream tFile(tFilename.c_str(), ios::out);
  if (! tFile.is_open()) {
    cout << "    !!! can not open file : " << tFilename << endl;
    exit(-2);
  }
  cout << "    Input file :" << iFilename << endl;
  ofstream iFile(iFilename.c_str(), ios::out);
  if (! iFile.is_open()) {
    cout << "    !!! can not open file : " << iFilename << endl;
    exit(-2);
  }
  cout << "    Output file:" << oFilename << endl;
  ofstream oFile(oFilename.c_str(), ios::out);
  if (! oFile.is_open()) {
    cout << "    !!! can not open file : " << oFilename << endl;
    exit(-2);
  }
  tFile << "----- relations -----------------------" << endl;
  for (unsigned i = 0; i < outputs.size(); i++) {
    tFile << i << " : " << outputs[i] << " = " << logics[outputs[i]]
          << endl;
  }
  tFile << "----- inputs -------------------------" << endl;
  for (unsigned i = 0; i < inputBits.size(); i++) {
    tFile << i << " : " << inputBits[i] << endl;
    iFile << i << "   " << inputNames[i] << endl;
  }
  tFile << "----- outputs ------------------------" << endl;
  for (unsigned i = 0; i < outputs.size(); i++) {
    tFile << i << " : " << outputs[i] << endl;
    tFile << "    " << expanded[i] << endl;
    tFile << "    " << expanded2[i] << endl;
    oFile << i << "   " << outputs[i] << endl;
  }
  tFile << "----- logics ------------------------" << endl;
  for (unsigned i = 0; i < outputs.size(); i++) {
    tFile << i << " : " << outputs[i] << endl;
    tFile << "    " << logics[outputs[i]] << endl;
    tFile << "    " << expanded[i] << endl;
    tFile << "    " << expanded2[i] << endl;
  }
  tFile.close();
  iFile.close();
  oFile.close();

  //...Summary for alg file...
  string aFilename = filename + ".alg";
  cout << "    alg file   :" << aFilename << endl;
  ofstream aFile(aFilename.c_str(), ios::out);
  if (! aFile.is_open()) {
    cout << "    !!! can not open file : " << aFilename << endl;
    exit(-3);
  }
  for (unsigned i = 0; i < expanded2.size(); i++) {
    aFile << i << " : ( " << expanded2[i] << " )" << endl;
  }
  aFile.close();

  //...C++ alg file...
  string cFilename = filename + ".cc";
  string funcname = filename;
  tofunc(funcname);
  cout << "    c++ file   :" << cFilename << endl;
  ofstream cFile(cFilename.c_str(), ios::out);
  if (! cFile.is_open()) {
    cout << "    !!! can not open file : " << cFilename << endl;
    exit(-3);
  }
  cFile << "namespace Belle2 {" << endl;
  cFile << "void " << funcname << "(bool * b, bool * i) {" << endl;
  for (unsigned i = 0; i < expandedcc.size(); i++) {
    cFile << expandedcc[i] << endl;
  }
  cFile << "}" << endl;
  cFile << "}" << endl;
  cFile.close();

  //...Summary for paw...
  string kFilename = filename + ".kumac";
  cout << "    kumac file :" << kFilename << endl;
  ofstream kFile(kFilename.c_str(), ios::out);
  if (! kFile.is_open()) {
    cout << "    !!! can not open file : " << kFilename << endl;
    exit(-3);
  }
  kFile << "macro main wait=wait baseRise=197 baseFall=293" << endl;
  for (unsigned i = 0; i < expanded.size(); i++) {
    string logic = expanded[i];
    kFile << "*...FTD " << i << "..." << endl;
    kFile << "*    " << logic << endl;
    kumacOutput(kFile, logic, outputs[i]);
    kFile << endl;
  }
  kFile << "return" << endl;
  kFile.close();

  return 0;
}

void
shrink(char b[800])
{
  char c[800];
  unsigned i = 0;
  unsigned j = 0;
  bool deleteSpace = true;
  while (b[i]) {
    char d = b[i++];
    char e = b[i];
    if (d == '\t') d = ' ';
    if (d == ';') d = ' ';
    if ((d == '|') && (e == '|')) d = ' ';

    const bool space = (d == ' ');
    if (deleteSpace && space)
      continue;
    if (space)
      deleteSpace = true;
    else
      deleteSpace = false;
    c[j] = d;
    ++j;
  }
  c[j] = 0;
  strcpy(b, c);
}

void
removeComments(char b[800])
{
  if ((b[0] == 0) || (b[1] == 0)) return;
  unsigned i = 1;
  while (b[i]) {
    char c = b[i - 1];
    char d = b[i];

    if ((c == '/') && (d == '/')) {
      b[i - 1] = 0;
      return;
    }
    i++;
  }
}

void
chopWord(char b[800], char c[800])
{
  c[0] = 0;
  unsigned i = 0;
  while (1) {
    c[i] = b[i];
    if ((b[i] == ' ') || (b[i] == 0)) {
      c[i] = 0;
      break;
    }
    ++i;
  }

  if (b[i] == 0) {
    b[0] = 0;
    return;
  }

//  for (unsigned j = i, k = 0; j++, k++; j < 800) {
  for (unsigned j = i, k = 0; j < 800; j++, k++) {
    b[k] = b[j];
    ++k;
    if (b[j] == 0)
      return;
  }
}

void
getInput(char b[800])
{
  char w[800];

  //...First word should be "reg"...
  chopWord(b, w);
  if (strcmp(w, "reg")) return;

  //...Next word should be the name of input, or bit width...
  unsigned b0 = 0;
  unsigned b1 = 0;
  chopWord(b, w);
  if (w[0] == '[') {

    //...One character...
    if (w[2] == ':') {
      b0 = w[3] - '0';
      b1 = w[1] - '0';
    } else {
      b0 = w[4] - '0';
      b1 = (w[1] - '0') * 10 + (w[2] - '0');
    }

    chopWord(b, w);
  }

  inputs.push_back(string(w));
  inputSizes.push_back(b1 - b0);

  if (b0 != b1) {
    if ((b1 - b0) < 10) {
      for (unsigned i = b0; i <= b1; i++) {
        char n[2];
        n[0] = '0' + i;
        n[1] = 0;
        inputBits.push_back(string(w) + "[" + string(n) + "]");
        inputNames.push_back(string(w) + string(n));
      }
    } else {
      for (unsigned i = b0; i <= b1; i++) {
        if (i < 10) {
          char n[2];
          n[0] = '0' + i;
          n[1] = 0;
          inputBits.push_back(string(w) + "[" + string(n) + "]");
          inputNames.push_back(string(w) + string(n));
        }
        //...Assuming less than 20...
        else {
          char n[3];
          n[0] = '1';
          n[1] = '0' + i - 10;
          n[2] = 0;
          inputBits.push_back(string(w) + "[" + string(n) + "]");
          inputNames.push_back(string(w) + string(n));
        }
      }
      if (DebugLevel) {
//              cout << inputNames.last() << endl;
      }
    }
  } else {
    inputBits.push_back(string(w));
    inputNames.push_back(string(w));
  }
}

void
getOutput(char b[800])
{
  char w[800];

  //...First word should be "reg"...
  chopWord(b, w);
  if (strcmp(w, "reg")) return;

  //...Next word should be the name...
  chopWord(b, w);

  outputs.push_back(string(w));
}

void
getLogic(char b[800])
{
  char w[800];

  //...First word should be the output name...
  string tmp = b;
  getLogicGND(tmp);
  strcpy(b, tmp.c_str());
  removeComments(b);
  chopWord(b, w);
  string name = w;
  chopWord(b, w);       // This must be '='.

  for (unsigned i = 0; i < outputs.size(); i++) {
    if (name == outputs[i]) {
      string logic = b;
      logics[name] = logic;
    }
  }
}

string
breakup(const string& logic)
{
  string s = logic;
  string sp = " ";

  //...Instert spaces...
//  for (string::size_type i = s.size(); i >= 0; i--) {
  int len = int(s.size());
  for (int i = len; i >= 0; i--) {
    char c = s[i];
    if ((c == '(') ||
        (c == ')') ||
        (c == '&') ||
        (c == '|') ||
        (c == '>') ||
        (c == '<') ||
        (c == '!') ||
        (c == '=')) {
      string d = s.substr(i, 1);

      if (i == 0) {
        s.insert(i + 1, sp);
      } else if (i == int(s.size())) {
        s.insert(i, sp);
      } else if (c == '=') {
        // assuming only "=="
        s.insert(i - 1, sp);
        s.insert(i + 2, sp);
      } else {
        s.insert(i, sp);
        s.insert(i + 2, sp);
      }

      if (DebugLevel) {
        cout << "i=" << i << ",d=[" << d << "]" << endl;
        cout << s << endl;
      }
    }
  }

  char w[800];
  strcpy(w, s.c_str());
  shrink(w);

  if (DebugLevel) {
    cout << "    breakup : in=[" << logic << "]" << endl;
    cout << "             out=[" << string(w) << "]" << endl;
  }

  return string(w);
}

string
decodeLogic(const string& logic)
{
  string s = logic;
  s = breakup(s);
  char c[800];
  strcpy(c, s.c_str());

  string f;
  char p0[800] = "";
  char p1[800] = "";
  while (1) {
    char cw[800];
    chopWord(c, cw);
    if (! cw[0]) {
      if (f.size() && (p1[0] != 0)) f += " ";
      f += p1;
      if (f.size() && (p0[0] != 0)) f += " ";
      f += p0;
      break;
    }

    if (DebugLevel) {
      cout << "p0=" << p0 << ",p1=" << p1 << endl;
    }

    //...'>'...
    if (p0[0] == '>') {

      //...Search for width...
      string name = p1;
      unsigned n = 999;
      for (unsigned i = 0; i < inputs.size(); i++) {
        if (inputs[i] == p1) {
          n = i;
          break;
        }
      }
      unsigned width = inputSizes[n] + 1;
      string x = "( ";
      if (cw[0] == '0') {
        char b = '0';
        for (unsigned i = 0; i < width; i++) {
          if (i) x += " | ";
          x += name + "[" + char(b + i) + "]";
        }
      } else if (cw[0] == '1') {
        char b = '0';
        for (unsigned i = 1; i < width; i++) {
          if (i > 1) x += " | ";
          x += name + "[" + char(b + i) + "]";
        }
      } else if (cw[0] == '2') {
        char b = '0';
        x = "(" + name + "[0] & " + name + "[1] )";
        for (unsigned i = 2; i < width; i++) {
          x += " | ";
          x += name + "[" + char(b + i) + "]";
        }
      } else if (cw[0] == '3') {
        char b = '0';
        for (unsigned i = 2; i < width; i++) {
          if (i > 2) x += " | ";
          x += name + "[" + char(b + i) + "]";
        }
      } else if (cw[0] == '4') {
        char b = '0';
        x = "(" + name + "[1] & " + name + "[2] )";
        x += "| (" + name + "[0] & " + name + "[2] )";
        for (unsigned i = 3; i < width; i++) {
          x += " | ";
          x += name + "[" + char(b + i) + "]";
        }
      } else {
        cout << "decodeLogic !!! [> with 4] is not supported"
             << endl;
        cout << "                " << logic << endl;
      }

      x += " )";

      if (DebugLevel) {
        cout << "    s=" << p1 << " " << p0 << " " << cw
             << endl;
        cout << "    name=" << name << ",width=" << width
             << endl;
        cout << "    x={" << x << "}" << endl;
      }

      strcpy(p1, x.c_str());
      strcpy(p0, "");
      strcpy(cw, "");
    }

    //...'<'...
    else if (p0[0] == '<') {
      cout << "decodeLogic !!! [> with 4] is not supported"
           << endl;
    }

    //...'='...
    else if (p0[0] == '=') {

      //...Search for width...
      string name = p1;
      unsigned n = 999;
      for (unsigned i = 0; i < inputs.size(); i++) {
        if (inputs[i] == p1) {
          n = i;
          break;
        }
      }
      unsigned width = inputSizes[n] + 1;
      string x = "( ";
      if (cw[0] == '0') {
        char b = '0';
        for (unsigned i = 0; i < width; i++) {
          if (i) x += " & ";
          x += "!" + name + "[" + char(b + i) + "]";
        }
      } else {
        cout << "decodeLogic !!! '== n' is not supported"
             << endl;
        cout << "                " << logic << endl;
      }

      if (DebugLevel) {
        cout << "    s=" << p1 << " " << p0 << " " << cw
             << endl;
        cout << "    name=" << name << ",width=" << width
             << endl;
        cout << "    x={" << x << "}" << endl;
      }

      x += " )";

      strcpy(p1, x.c_str());
      strcpy(p0, "");
      strcpy(cw, "");
    }

    if (f.size() && (p1[0] != 0)) f += " ";
    f += p1;
    strcpy(p1, p0);
    strcpy(p0, cw);

  }

  Term t(f);
  t.expand();

  return t.expanded();
}

Term::Term(const string& a)
  : _sentence(a)
{
  char b[800];
  strcpy(b, _sentence.c_str());

  char w[800];
  while (1) {
    chopWord(b, w);
    if (w[0] == 0) break;
    _words.push_back(string(w));
  }

  //...Create term candidates...
  unsigned inP = 0;
  vector<string> termCandidates;
  string t;
  for (unsigned i = 0; i < _sentence.size(); i++) {
    char c = _sentence[i];

    if (c == '(') {
      t += c;
      ++inP;
      continue;
    } else if (c == ')') {
      t += c;
      --inP;
      continue;
    }

    if (inP) {
      t += c;
      continue;
    }

    if (c == '|') {
      termCandidates.push_back(t);
      t = "";
      continue;
    }

    t += c;
  }
  termCandidates.push_back(t);

  //...Check term candidates...
  if (termCandidates.size() > 1) {
    for (unsigned i = 0; i < termCandidates.size(); i++) {
      Term x(termCandidates[i]);
      _terms.push_back(x);
    }

//  cout << "Term list:#=" << _terms.size() << endl;
//  for (unsigned i = 0; i < _terms.size(); i++) {
//      cout << i << ":" << _terms[i].sentence() << endl;
//  }
  }
}

Term::~Term()
{
}

string
Term::sentence(void) const
{
  return _sentence;
}

void
Term::expand(void)
{
  _expanded = "";

  //...Have sub-terms...
  if (_terms.size() > 1) {
    for (unsigned i = 0; i < _terms.size(); i++) {
      _terms[i].expand();
      if (i)
        _expanded += " | ";
      _expanded += _terms[i].expanded();
    }
    return;
  }

  //...Expand...
  _expanded = _sentence;
  while (1) {
    unsigned a = 0;
    unsigned b = 0;
    for (unsigned i = 0; i < _expanded.size(); i++) {
      char c = _expanded[i];
      if (c == '(') {
        a = i;
      } else if (c == ')') {
        b = i;
        break;
      }
    }

    //...No ()...
    if (a == b) return;

    //...necessary to expand...
    string s0 = _expanded.substr(0, a - 1);
    if (a == 0) s0 = "";
    string p = _expanded.substr(a + 1, b - a - 1);
    string s1 = _expanded.substr(b + 1);

//  cout << "a,b=" << a << "," << b << endl;
//  cout << "    s0=" << s0 << endl;
//  cout << "    p =" << p << endl;
//  cout << "    s1=" << s1 << endl;

    //...divide p into two...
    unsigned a1 = 0;
    for (unsigned i = 0; i < p.size(); i++) {
      char c = p[i];
      if (c == '|') {
        a1 = i;
        break;
      }
    }
    if (a1) {
      string p0 = p.substr(0, a1 - 1);
      string p1 = p.substr(a1 + 1);
      _expanded = s0 + p0 + s1 + " | " + s0 + " (" + p1 + ") " + s1;

      Term tmp(_expanded);
      tmp.expand();
      _expanded = tmp.expanded();
    } else {
      _expanded = s0 + p + s1;
    }

    // cout << "--->" << endl;
    // cout << _expanded << endl;
    // cout << "<---" << endl;
  }
}

string
Term::expanded(void)
{
  return _expanded;
}

void
shrink(string& s)
{
  string t;
  bool deleteSpace = true;
  for (unsigned i = 0; i < s.size(); i++) {
    char d = s[i];
    char e = 0;
    if ((i + 1) < s.size())
      e = s[i + 1];

    if (d == '\t') d = ' ';
    if (d == ';') d = ' ';
    if ((d == '|') && (e == '|')) d = ' ';

    const bool space = (d == ' ');
    if (deleteSpace && space)
      continue;
    if (space)
      deleteSpace = true;
    else
      deleteSpace = false;

    t += d;
  }

  s = t;
}

void
cosmetic(string& s)
{
  for (unsigned i = 0; i < s.size(); i++) {
    char d = s[i];
    char e = 0;
    if ((i + 1) < s.size())
      e = s[i + 1];

    if (d == '&')
      s[i] = '*';
    else if (d == '|')
      s[i] = '+';
    else if ((d == '!') && (e == ' '))
      s.erase(i + 1, 1);
  }
}

void
getLogicGND(string& s)
{
//    cout << "s0=" << s << endl;
  string g = "= 0";
  string::size_type p = s.find(g);
  if (p != string::npos)
    s.replace(p, 3, "= GND", 0, 5);
//    cout << "s1=" << s << endl;
}

void
kumacOutput(ofstream& f, string logic, const string& ftd)
{
  const string tab = "    ";
  bool first = true;

  //...Check how many ORs...
  unsigned nO = 0;
  for (unsigned i = 0; i < logic.size(); i++)
    if (logic[i] == '|')
      ++nO;

  unsigned n = 0;
  while (1) {
    string t;
    chopWord(logic, t);
    if (t.empty()) break;

    if (t == "|")
      first = true;

    if (first) {
      f << "[wait]" << endl;
      f << tab << "zone 1 6 1" << endl;
      f << tab << "title '" << ftd << " " << n + 1 << "/" << nO + 1 <<
        "'" << endl;
      first = false;
      ++n;
    }

    for (unsigned i = 0; i < inputBits.size(); i++) {
      if (t == inputBits[i]) {
        f << tab << "set hcol 2" << endl;
        f << tab << "id=[baseRise]+" << i << ";"
          << "hi/pl [id]" << endl;
        f << tab << "set hcol 4" << endl;
        f << tab << "id=[baseFall]+" << i << ";"
          << "hi/pl [id] s" << endl;
        f << tab << "atit '" << t << "' ' '" << endl;
        first = false;
        break;
      }
    }
  }
}

void
chopWord(string& b, string& c)
{
  c = "";
  unsigned i = 0;
  while (1) {
    if ((b[i] == ' ') || (b[i] == 0))
      break;

    c += b[i];
    ++i;
  }

  int l = b.size() - i - 1;
  if (l > 0)
    b = b.substr(i + 1, l);
  else
    b = "";
}

void
tocc(string& s)
{
  while (s.find("*") != string::npos) {
    const unsigned n = s.size();
    for (unsigned i = 0; i < n; i++)
      if (s[i] == '*')
        s.replace(i, 2, "&& ", 0, 3);
  }
  while (s.find("+") != string::npos) {
    const unsigned n = s.size();
    for (unsigned i = 0; i < n; i++)
      if (s[i] == '+')
        s.replace(i, 2, ") || ( ", 0, 6);
  }
}

void
tofunc(string& s)
{
  const unsigned n = s.size();
  for (unsigned i = 0; i < n; i++)
    if (s[i] == '.')
      s.replace(i, 1, "_", 0, 1);
}
