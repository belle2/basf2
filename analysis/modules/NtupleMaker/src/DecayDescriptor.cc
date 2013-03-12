/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald                                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/modules/NtupleMaker/DecayDescriptor.h>
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <boost/spirit/include/classic.hpp>
#include <boost/bind.hpp>
#include <functional>
#include <TParticlePDG.h>

using namespace std;
using namespace boost::spirit::classic;
using namespace Belle2;

const DecayDescriptor& DecayDescriptor::m_NULL = DecayDescriptor();

DecayDescriptor::DecayDescriptor() :
  m_daughters(),
  m_strName(""),
  m_iPDGCode(0),
  m_isSelected(false),
  m_isFixed(false)
{
}

DecayDescriptor::DecayDescriptor(string strName, int iPDGCode) :
  m_daughters(),
  m_strName(strName),
  m_iPDGCode(iPDGCode),
  m_isSelected(false),
  m_isFixed(false)
{
}

DecayDescriptor::DecayDescriptor(const DecayDescriptor& other) :
  m_daughters(other.m_daughters),
  m_strName(other.m_strName),
  m_iPDGCode(other.m_iPDGCode),
  m_isSelected(other.m_isSelected),
  m_isFixed(other.m_isFixed)
{
}


void DecayDescriptor::append(const DecayDescriptor& daughter)
{
  if (m_isFixed) {
    printf("DecayDescriptor::append - This DecayDescriptor is already used as daughter of another DecayDescritor!\n");
    return;
  }
  m_daughters.push_back(daughter);
}

bool DecayDescriptor::init(const string strDecayString)
{

  if (m_isFixed) {
    B2WARNING("Could not initialise DecayDescriptor from decay string");
    return false;
  }

  string strThis;
  vector<string> strDaughters;
  bool isSelected = false;

  rule<> _ARROW = *space_p >> strlit<>("->")  >> *space_p;
  //rule<> _ARROW = *space_p >> chlit<>('-')  >> chlit<>('>') >> *space_p;
  rule<> _SELECTOR = *space_p >> chlit<>('^');
  rule<> _RESERVED = _SELECTOR | chlit<>('(') | chlit<char>(')') | space_p | _ARROW;
  rule<> _PARTICLE = +(anychar_p - _RESERVED);
  rule<> _LEFT = *space_p >> *_SELECTOR[assign_a(isSelected, true)] >> _PARTICLE[assign_a(strThis)] >> *space_p;
  rule<> _SELPARTICLE = *space_p >> *_SELECTOR >> _PARTICLE >> *space_p;
  rule<> _DAUGHTER = (_SELPARTICLE >> chlit<>('(') >> _ARROW >> +_SELPARTICLE >> chlit<>(')') >> *space_p) | _SELPARTICLE;
  rule<> _RIGHT = +_DAUGHTER[push_back_a(strDaughters)];
  rule<> _DECAY = (_LEFT >> _ARROW >> _RIGHT) | (_LEFT >> chlit<char>('(') >> _ARROW >> _RIGHT >> chlit<char>(')') >> *space_p) | _LEFT;

  bool isParseOK = parse(strDecayString.c_str(), _DECAY).full;

  if (!isParseOK) {
    B2WARNING("Could not parse decay descriptor string!" << strDecayString);
    return false;
  }

  m_strName = strThis;
  m_isSelected = isSelected;

  for (vector<string>::iterator i = strDaughters.begin(); i != strDaughters.end(); ++i) {
    DecayDescriptor daughter;
    bool isDaughterOK = daughter.init(*i);
    if (!isDaughterOK) return false;
    append(const_cast<const DecayDescriptor&>(daughter));
  }
  return true;
}

vector<const Particle*> DecayDescriptor::getSelectionParticles(const Particle* particle)
{
  vector<const Particle*> selparticles;
  if (m_isSelected) selparticles.push_back(particle);
  int nDaughters = m_daughters.size();
  for (int iDaughter = 0; iDaughter < nDaughters; ++iDaughter) {
    const Particle* daughter = particle->getDaughter(iDaughter);
    if (!daughter) B2WARNING("Could not find daughter!");
    vector<const Particle*> seldaughters = m_daughters[iDaughter].getSelectionParticles(daughter);
    selparticles.insert(selparticles.end(), seldaughters.begin(), seldaughters.end());
  }
  return selparticles;
}

vector<string> DecayDescriptor::getSelectionNames()
{
  vector<string> strNames;
  if (m_isSelected) strNames.push_back(getName());
  for (vector<DecayDescriptor>::iterator i = m_daughters.begin(); i != m_daughters.end(); ++i) {
    vector<string> strDaughterNames = i->getSelectionNames();
    int nDaughters = strDaughterNames.size();
    for (int iDaughter = 0; iDaughter < nDaughters; iDaughter++) {
      strDaughterNames[iDaughter] = getName() + "_" + strDaughterNames[iDaughter];
    }
    strNames.insert(strNames.end(), strDaughterNames.begin(), strDaughterNames.end());
  }

  // search for multiple occurrence of the same name and then distinguish by attaching a number
  for (vector<string>::iterator itName = strNames.begin(); itName != strNames.end(); ++itName) {
    if (count(itName, strNames.end(), *itName) == 1) continue;
    // multiple occurrence found!
    string strNameOld = *itName;
    vector<string>::iterator itOccurrence = strNames.begin();
    int iOccurrence = 0;
    while (iOccurrence <= 10) {
      // find next occurence of the identical particle name defined in DecayDescriptor
      itOccurrence = find(itOccurrence, strNames.end(), strNameOld);
      // stop, if nothing found
      if (itOccurrence == strNames.end()) break;
      // create new particle name by attaching a number
      string strNameNew = strNameOld + boost::lexical_cast<string>(iOccurrence);
      // ceck if the new particle name exists already, if not, then it is OK to use it
      if (count(strNames.begin(), strNames.end(), strNameNew) == 0) {
        *itOccurrence = strNameNew;
        ++itOccurrence;
      }
      iOccurrence++;
    }
    if (iOccurrence == 10) {
      printf("DecayDescriptor::getSelectionNames - Something is wrong! More than 10x the same name!\n");
      break;
    }
  }
  return strNames;
}
