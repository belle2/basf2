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
#include <analysis/utility/PSelectorFunctions.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>

#include <iostream>
#include <iomanip>
#include <math.h>
#include <vector>
#include <string>
#include <sstream>

using namespace std;
using namespace Belle2::analysis;

namespace Belle2 {

  std::map<std::string, FunctionDescr>  PSelector::m_functionList;

  PSelector::PSelector()
  {
    if (m_functionList.empty()) initialize();
  }

  PSelector::~PSelector()
  {
  }

  void PSelector::initialize()
  {

    addFunction("p", "momentum magnitude", particleP);
    addFunction("px", "momentum component x", particlePx);
    addFunction("py", "momentum component y", particlePy);
    addFunction("pz", "momentum component z", particlePz);
    addFunction("pt", "transverse momentum", particlePt);
    addFunction("cosTheta", "momentum cosine of polar angle", particleCosTheta);
    addFunction("cth", "momentum cosine of polar angle", particleCosTheta);
    addFunction("phi", "momentum azimuthal angle in degrees", particlePhi);

    addFunction("p*", "CMS momentum magnitude", particlePStar);
    addFunction("px*", "CMS momentum component x", particlePxStar);
    addFunction("py*", "CMS momentum component y", particlePyStar);
    addFunction("pz*", "CMS momentum component z", particlePzStar);
    addFunction("pt*", "CMS transverse momentum", particlePtStar);
    addFunction("cosTheta*", "CMS momentum cosine of polar angle", particleCosThetaStar);
    addFunction("cth*", "CMS momentum cosine of polar angle", particleCosThetaStar);
    addFunction("phi*", "CMS momentum azimuthal angle in degrees", particlePhiStar);

    addFunction("dx", "x in respect to IP", particleDX);
    addFunction("dy", "y in respect to IP", particleDY);
    addFunction("dz", "z in respect to IP", particleDZ);
    addFunction("dr", "transverse distance in respect to IP", particleDRho);

    addFunction("M", "mass", particleMass);
    addFunction("dM", "mass minus nominal mass", particleDMass);
    addFunction("Q", "released energy in decay", particleQ);
    addFunction("dQ", "released energy in decay minus nominal one", particleDQ);
    addFunction("Mbc", "beam constrained mass", particleMbc);
    addFunction("deltaE", "energy difference", particleDeltaE);

    addFunction("eid", "electron identification probability", particleElectronId);
    addFunction("muid", "muon identification probability", particleMuonId);
    addFunction("piid", "pion identification probability", particlePionId);
    addFunction("Kid", "kaon identification probability", particleKaonId);
    addFunction("prid", "proton identification probability", particleProtonId);

    addFunction("chiProb", "chi^2 probability of the fit", particlePvalue);
    addFunction("childs", "number of daughter particles", particleNchilds);
    addFunction("flavor", "flavor type of decay (0=unflavored, 1=flavored)",
                particleFlavorType);


    cout << "--------------------------------------------------------\n";
    //    cout<<"PSelector initialized with "<< m_functionList.size()
    //  << " selection variables:" <<endl;
    cout << "Modular analysis: list of available selection variables:\n";
    cout << "--------------------------------------------------------\n";
    listVariables();
    cout << "--------------------------------------------------------\n";

  }

  void PSelector::addFunction(std::string varName, std::string varDescription,
                              double(*fun)(const Particle*))
  {
    std::map<std::string, FunctionDescr>::iterator it = m_functionList.find(varName);
    if (it == m_functionList.end()) {
      m_functionList[varName] = make_pair(varDescription, fun);
    } else {
      cout << " ***error PSelector::addFunction: variable " << varName <<
           " already in the list - ignored" << endl;
    }
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

    std::map<std::string, FunctionDescr>::iterator it;
    it = m_functionList.find(varName);
    if (it == m_functionList.end()) {
      cout << str << " ***undefined variable: " << varName << endl;
      return false;
    }
    FunctionDescr fd = it->second;
    SelectionCriteria selection(fd.second, childIndex, ranges);
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
    for (std::map<std::string, FunctionDescr>::iterator it = m_functionList.begin();
         it != m_functionList.end(); ++it) {
      cout << setw(12) << it->first << "  " << (it->second).first << endl;
    }
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


