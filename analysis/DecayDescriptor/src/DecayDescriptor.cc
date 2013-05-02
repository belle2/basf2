/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald                                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <algorithm>
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/spirit/include/classic.hpp>
#include <framework/core/Environment.h>
#include <evtgen/EvtGenBase/EvtPDL.hh>

using namespace std;
using namespace boost::spirit::classic;
using namespace boost::algorithm;
using namespace Belle2;

const DecayDescriptor& DecayDescriptor::m_NULL = DecayDescriptor();

DecayDescriptor::DecayDescriptor() :
  m_daughters(),
  m_strName(""),
  m_strTag("default"),
  m_iPDGCode(0),
  m_isSelected(false),
  m_isWithCC(false),
  m_isInclusive(false),
  m_isFixed(false)
{
}

DecayDescriptor::DecayDescriptor(const DecayDescriptor& other) :
  m_daughters(other.m_daughters),
  m_strName(other.m_strName),
  m_strTag(other.m_strTag),
  m_iPDGCode(other.m_iPDGCode),
  m_isSelected(other.m_isSelected),
  m_isWithCC(other.m_isWithCC),
  m_isInclusive(other.m_isInclusive),
  m_isFixed(other.m_isFixed)
{
}

string DecayDescriptor::getNameSimple()
{
  string strNameSimple(m_strName);
  erase_all(strNameSimple, "anti-");
  erase_all(strNameSimple, "+");
  erase_all(strNameSimple, "-");
  replace_all(strNameSimple, "*", "ST");
  return strNameSimple;
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
  B2INFO("Parsing Descriptor: " << strDecayString);

  if (m_isFixed) {
    B2WARNING("Could not initialise DecayDescriptor from decay string");
    return false;
  }
  string strTag;
  vector<string> strDaughters;

  // include charge conjugated decay
  rule<> _CHCONJ = strlit<>("CC");
  // exact match of decay
  rule<> _ARROW_EXACT = *space_p >> strlit<>("->")  >> *space_p;
  // intermediate resonances ignored
  rule<> _ARROW_NORES = *space_p >> strlit<>("-->")  >> *space_p;
  // additional photons ignored
  rule<> _ARROW_IGGAM = *space_p >> strlit<>("=>")  >> *space_p;
  // intermediate resonances and additional photons ignored
  rule<> _ARROW_NORES_IGGAM = *space_p >> strlit<>("==>")  >> *space_p;
  rule<> _ARROW = _ARROW_EXACT | _ARROW_NORES | _ARROW_IGGAM | _ARROW_NORES_IGGAM;

  // plus any additional particles
  rule<> _INCLUSIVE = *space_p >> strlit<>("...") >> *space_p;
  // select the following particle
  rule<> _SELECTOR = chlit<>('^');

  // forbidden character sequences for particle and tag names
  rule<> _RESERVED = _SELECTOR | chlit<>('(') | chlit<char>(')') | chlit<>('{') | chlit<>('}') | space_p | _ARROW | _CHCONJ | _INCLUSIVE;
  // name of particle or tag
  rule<> _NAME = +(anychar_p - _RESERVED);
  rule<> _TAG = chlit<>('{') >> _NAME >> chlit<>('}') >> *space_p;
  // particle on the left side (save properties)
  rule<> _LPARTICLE = *space_p >> *_SELECTOR[assign_a(m_isSelected, true)] >> _NAME[assign_a(m_strName)] >> *_TAG[assign_a(m_strTag)];
  // particle on the right side (will be evaluated by daughter decay descriptor)
  rule<> _RPARTICLE = *space_p >> *_SELECTOR >> _NAME >> *_TAG;

  // everything that can be on the right side
  //rule<> _DAUGHTER = _RPARTICLE | (*space_p >> chlit<>('(') >> _RPARTICLE >> *_CHCONJ >> _ARROW >> +_RPARTICLE >> *_INCLUSIVE >> chlit<>(')') >> *space_p);
  rule<> _DAUGHTER = _RPARTICLE | (*space_p >> chlit<>('(') >> _RPARTICLE >> _ARROW >> +_RPARTICLE >> *_INCLUSIVE >> chlit<>(')') >> *space_p);

  rule<> _RIGHT = +_DAUGHTER[push_back_a(strDaughters)] >> *_INCLUSIVE[assign_a(m_isInclusive, true)];

  //rule<> _DECAYBASIC = _LPARTICLE >> *_CHCONJ[assign_a(m_isWithCC, true)] >> _ARROW >> _RIGHT;
  rule<> _DECAYBASIC = _LPARTICLE >> _ARROW >> _RIGHT;
  rule<> _DECAY = _DECAYBASIC | (*space_p >> chlit<char>('(') >> _DECAYBASIC >> chlit<char>(')') >> *space_p) | _LPARTICLE;

  bool isParseOK = parse(strDecayString.c_str(), _DECAY).full;

  if (!isParseOK) {
    B2WARNING("Could not parse decay descriptor string! Stopped @ " << parse(strDecayString.c_str(), _DECAY).stop);
    return false;
  }

  if (EvtPDL::entries() == 0) {
    string strEvtPDL = Environment::Instance().getExternalsPath() + "/share/evtgen/evt.pdl";
    EvtPDL evtpdl;
    evtpdl.read(strEvtPDL.c_str());
  }
  m_iPDGCode = EvtPDL::getLundKC(EvtPDL::getId(m_strName));
  B2INFO(m_strName << "   ------>   " << m_iPDGCode << " " << m_strTag << " SEL = " << m_isSelected << " CC = " << m_isWithCC << " INCL = " << m_isInclusive);

  for (unsigned int i = 0; i < strDaughters.size(); i++) B2INFO(strDaughters[i]);
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
  if (m_isSelected) strNames.push_back(getNameSimple());
  for (vector<DecayDescriptor>::iterator i = m_daughters.begin(); i != m_daughters.end(); ++i) {
    vector<string> strDaughterNames = i->getSelectionNames();
    int nDaughters = strDaughterNames.size();
    for (int iDaughter = 0; iDaughter < nDaughters; iDaughter++) {
      strDaughterNames[iDaughter] = getNameSimple() + "_" + strDaughterNames[iDaughter];
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
