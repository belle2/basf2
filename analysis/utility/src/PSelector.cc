/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/utility/PSelector.h>
#include <analysis/utility/VariableManager.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>

#include <iostream>
#include <math.h>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

namespace Belle2 {


  PSelector::PSelector()
  {
  }

  PSelector::~PSelector()
  {
  }

  bool PSelector::select(const Particle* particle)
  {

    if (!particle) return true;

    for (unsigned int i = 0; i < m_selectionList.size(); i++) {
      const Particle* part = particle;
      for (unsigned k = 0; k < m_selectionList[i].childIndex.size(); k++) {
        part = part->getDaughter(m_selectionList[i].childIndex[k]);
        if (!part) break;
      }
      if (!part) {
        std::string str;
        listCuts(str);
        cout << "***warning: non-existing child in " << str << endl;
        continue;
      }
      double x = m_selectionList[i].function(part);
      if (!selectLevel0(x, m_selectionList[i].ranges)) return false;
    }
    return true;
  }


  bool PSelector::selectLevel0(double x, const SelRangeVector& ranges)
  {
    for (unsigned int i = 0; i < ranges.size(); i++) {
      if (intheRange(x, ranges[i])) return true;
    }
    return false;
  }


  bool PSelector::intheRange(double x, const SelectionRange& range)
  {
    if (range.lmin) {if (x < range.xmin) return false;}
    if (range.lmax) {if (x > range.xmax) return false;}
    return true;
  }


  bool PSelector::addSelection(std::string str) // false for syntax error
  {
    if (str.empty()) return true;

    stringstream ss, tt;
    string substr, varName;
    ss << str;
    ss >> substr;
    string fullName = substr;

    size_t i = fullName.find("..");
    if (i != string::npos) {
      cout << str << " ***syntax error in variable name: " << fullName << endl;
      return false;
    }
    i = substr.find(".");
    while (i != string::npos) {
      substr.replace(i, 1, " ");
      i = substr.find(".");
    }
    tt << substr;
    tt >> varName;

    unsigned int child;
    vector <unsigned int> childIndex;
    while (!tt.eof()) {
      tt >> child;
      if (tt.fail()) {
        cout << str << " ***syntax error in variable name: " << fullName << endl;
        return false;
      }
      if (child == 0) {
        cout << str << " ***syntax error in variable name: " << fullName;
        cout << " daughter numbers are 1-based" << endl;
        return false;
      }
      child--; // convert to 0-based index
      childIndex.push_back(child);
    }

    SelRangeVector ranges;
    while (!ss.eof()) {
      ss >> substr;
      if (ss.fail()) break;
      bool ok = appendRange(fullName, substr, ranges);
      if (!ok) {
        cout << str << " ***syntax error: " << substr << endl;
        return false;
      }
    }
    if (ranges.empty()) {
      cout << str << " ***no range given, ignored" << endl;
      return true;
    }

    const VariableManager::Var* var = VariableManager::Instance().getVariable(varName);
    if (!var) {
      cout << str << " ***undefined variable: " << varName << endl;
      return false;
    }
    SelectionCriteria selection(var->function, childIndex, ranges);
    m_selectionList.push_back(selection);

    return true;
  }


  bool PSelector::appendRange(string varName, string str, SelRangeVector& Ranges)
  {
    if (str.empty()) return false;

    SelectionRange range(varName);

    size_t i = str.find("+-");
    if (i == string::npos) { // "+-" not found: try to read
      bool ok = getRange(str, range);
      if (ok) Ranges.push_back(range);
      return ok;
    }

    // "+-" found, must be the leftmost
    if (i != 0) return false;

    str.replace(i, 2, "");

    i = str.find(":");
    if (i != string::npos) { // found, form "+-(A:B)"
      size_t i1 = str.find("(");
      size_t i2 = str.find(")");
      if (i1 == string::npos || i2 == string::npos) return false;
      str.replace(i1, 1, ""); i2--;
      str.replace(i2, 1, "");
      bool ok = getRange(str, range);
      if (ok) {
        SelectionRange swappedRange(range.varName);
        swappedRange.xmin = -range.xmax;
        swappedRange.lmin = range.lmax;
        swappedRange.xmax = -range.xmin;
        swappedRange.lmax = range.lmin;
        Ranges.push_back(swappedRange);
        Ranges.push_back(range);
      }
      return ok;
    } else { // form "+-(A)" or "+-A"
      size_t i1 = str.find("(");
      size_t i2 = str.find(")");
      if (i1 != string::npos && i2 != string::npos) {
        str.replace(i1, 1, ""); i2--;
        str.replace(i2, 1, "");
      }
      bool ok = getRange(str, range);
      if (ok) {
        range.xmin = - range.xmin;
        Ranges.push_back(range);
      }
      return ok;
    }
  }


  bool PSelector::getRange(string str, SelectionRange& range)
  // forms "A:B", "A:", ":B", "A", otherwise false
  {
    size_t i = str.find(":");
    if (i == string::npos) { // not found: try to read form "A"
      stringstream ss;
      double x;
      ss << str;
      ss >> x;
      if (ss.fail()) return false;
      if (ss.good()) return false;
      range.xmin = x;
      range.xmax = x;
      range.lmin = true;
      range.lmax = true;
      return true;
    }

    if (i == 0) { // found at left: try to read form ":B"
      str.replace(i, 1, "");
      stringstream ss;
      double x;
      ss << str;
      ss >> x;
      if (ss.fail()) return false;
      if (ss.good()) return false;
      range.xmax = x;
      range.lmax = true;
      return true;
    }

    if (i == str.size() - 1) { // found at right: try to read form "A:"
      str.replace(i, 1, "");
      stringstream ss;
      double x;
      ss << str;
      ss >> x;
      if (ss.fail()) return false;
      if (ss.good()) return false;
      range.xmin = x;
      range.lmin = true;
      return true;
    }

    // found inside: try to read form "A:B"
    str.replace(i, 1, " ");
    stringstream ss;
    double x1, x2;
    ss << str;
    ss >> x1;
    ss >> x2;
    if (ss.fail()) return false;
    if (ss.good()) return false;
    range.xmin = x1;
    range.xmax = x2;
    range.lmin = true;
    range.lmax = true;
    return true;
  }


  void PSelector::listVariables() const
  {
    VariableManager::Instance().printList();
  }


  void PSelector::listCuts() const
  {
    cout << "Selection criteria:" << endl;
    for (unsigned int i = 0; i < m_selectionList.size(); i++) {
      if (i != 0) cout << " and ";
      unsigned int size = m_selectionList[i].ranges.size();
      if (size > 1) cout << "(";
      for (unsigned int k = 0; k < size; k++) {
        if (k != 0) cout << " or ";
        m_selectionList[i].ranges[k].print();
      }
      if (size > 1) cout << ")";
    }
    cout << endl;
  }


  void PSelector::listCuts(std::string& str) const
  {
    stringstream ss;

    for (unsigned int i = 0; i < m_selectionList.size(); i++) {
      if (i != 0) ss << " and ";
      unsigned int size = m_selectionList[i].ranges.size();
      if (size > 1) ss << "(";
      for (unsigned int k = 0; k < size; k++) {
        if (k != 0) ss << " or ";
        m_selectionList[i].ranges[k].print(ss);
      }
      if (size > 1) ss << ")";
    }
    std::getline(ss, str);
  }


} // end Belle2 namespace


