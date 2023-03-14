/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <analysis/DecayDescriptor/DecayString.h>
#include <analysis/DecayDescriptor/DecayStringDecay.h>
#include <analysis/DecayDescriptor/DecayStringGrammar.h>
#include <analysis/utility/EvtPDLUtil.h>
#include <analysis/dataobjects/Particle.h>

#include <analysis/utility/AnalysisConfiguration.h>

#include <mdst/dataobjects/MCParticle.h>

#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <TDatabasePDG.h>

#include <boost/variant/get.hpp>
#include <boost/spirit/include/qi.hpp>
#include <algorithm>
#include <set>
#include <utility>

using namespace Belle2;
using namespace std;

const DecayDescriptor& DecayDescriptor::s_NULL = DecayDescriptor();

DecayDescriptor::DecayDescriptor() :
  m_mother(),
  m_iDaughter_p(-1),
  m_daughters(),
  m_properties(0),
  m_isNULL(false),
  m_isInitOK(false)
{
}

bool DecayDescriptor::init(const std::string& str)
{
  // The decay string grammar
  DecayStringGrammar<std::string::const_iterator> g;
  DecayString s;
  std::string::const_iterator iter = str.begin();
  std::string::const_iterator end = str.end();
  bool r = phrase_parse(iter, end, g, boost::spirit::unicode::space, s);
  if (!r || iter != end) return false;
  return init(s);
}

bool DecayDescriptor::init(const DecayString& s)
{
  // The DecayString is a hybrid, it can be
  // a) DecayStringParticleList
  // b) DecayStringDecay

  if (const DecayStringParticle* p = boost::get< DecayStringParticle >(&s)) {
    m_isInitOK = m_mother.init(*p);
    if (!m_isInitOK) {
      B2WARNING("Could not initialise mother particle " << p->m_strName);
      return false;
    }
    return true;
  } else if (const DecayStringDecay* d = boost::get< DecayStringDecay > (&s)) {
    // Initialise list of mother particles
    m_isInitOK = m_mother.init(d->m_mother);
    if (!m_isInitOK) {
      B2WARNING("Could not initialise mother particle " << d->m_mother.m_strName);
      return false;
    }

    // Identify arrow type
    if (d->m_strArrow == "->") {
      m_properties |= Particle::PropertyFlags::c_IsIgnoreRadiatedPhotons ;
      m_properties |= Particle::PropertyFlags::c_IsIgnoreIntermediate;
    } else if (d->m_strArrow == "=norad=>") {
      m_properties |= Particle::PropertyFlags::c_IsIgnoreIntermediate;
    } else if (d->m_strArrow == "=direct=>") {
      m_properties |= Particle::PropertyFlags::c_IsIgnoreRadiatedPhotons;
    } else if (d->m_strArrow == "=exact=>") {
      // do nothing
    } else {
      B2WARNING("Unknown arrow: " << d->m_strArrow);
      m_isInitOK = false;
      return false;
    }

    // Initialise list of daughters
    if (d->m_daughters.empty()) {
      m_isInitOK = false;
      return false;
    }
    int nDaughters = d->m_daughters.size();
    for (int iDaughter = 0; iDaughter < nDaughters; iDaughter++) {
      DecayDescriptor daughter;
      m_isInitOK = daughter.init(d->m_daughters[iDaughter]);
      if (!m_isInitOK) {
        B2WARNING("Could not initialise daughter!");
        return false;
      }
      m_daughters.push_back(daughter);
    }

    // Initialise list of keywords
    // For neutrino
    if ((std::find(d->m_keywords.begin(), d->m_keywords.end(), "?nu")) !=  d->m_keywords.end()) {
      m_properties |= Particle::PropertyFlags::c_IsIgnoreNeutrino;
    }
    // For gamma
    if ((std::find(d->m_keywords.begin(), d->m_keywords.end(), "?gamma")) != d->m_keywords.end()) {
      m_properties |= Particle::PropertyFlags::c_IsIgnoreGamma;
    }
    // For massive FSP
    if ((std::find(d->m_keywords.begin(), d->m_keywords.end(), "...")) != d->m_keywords.end()) {
      m_properties |= Particle::PropertyFlags::c_IsIgnoreMassive;
    }
    // For brems photons
    if ((std::find(d->m_keywords.begin(), d->m_keywords.end(), "?addbrems")) != d->m_keywords.end()) {
      m_properties |= Particle::PropertyFlags::c_IsIgnoreBrems;
    }

    return true;
  }
  m_isInitOK = false;
  return false;
}

template <class T>
int DecayDescriptor::match(const T* p, int iDaughter_p)
{
  // this DecayDescriptor was not matched or
  // it is not the daughter of another DecayDescriptor
  m_iDaughter_p = -1;

  if (!p) {
    B2WARNING("NULL pointer provided instead of particle.");
    return 0;
  }

  int iPDGCode_p = 0;
  if (const auto* part_test = dynamic_cast<const Particle*>(p))
    iPDGCode_p = part_test->getPDGCode();
  else if (const auto* mc_test = dynamic_cast<const MCParticle*>(p))
    iPDGCode_p = mc_test->getPDG();
  else {
    B2WARNING("Template type not supported!");
    return 0;
  }

  int iPDGCodeCC_p = TDatabasePDG::Instance()->GetParticle(iPDGCode_p)->AntiParticle()->PdgCode();
  int iPDGCode_d = m_mother.getPDGCode();
  if (abs(iPDGCode_d) != abs(iPDGCode_p)) return 0;
  int iCC = 0;
  if (iPDGCode_p == iPDGCodeCC_p) iCC = 3;
  else if (iPDGCode_d == iPDGCode_p) iCC = 1;
  else if (iPDGCode_d == iPDGCodeCC_p) iCC = 2;

  const std::vector<T*> daughterList = p->getDaughters();
  int nDaughters_p = daughterList.size();

  // 1st case: the descriptor has no daughters => nothing to check
  if (getNDaughters() == 0) {
    m_iDaughter_p = iDaughter_p;
    return iCC;
  }

  // 2nd case: the descriptor has daughters, but not the particle
  // => that is not allowed!
  if (nDaughters_p == 0) return 0;

  // 3rd case: the descriptor and the particle have daughters
  // There are two cases that can happen when matching the
  // DecayDescriptor daughters to the particle daughters:
  // 1. The match is unambiguous -> no problem
  // 2. Multiple particle daughters match the same DecayDescriptor daughter
  // -> in the latter case the ambiguity is resolved later

  // 1. DecayDescriptor -> Particle relation for the cases where only one particle matches
  vector< pair< int, int > > singlematch;
  // 2. DecayDescriptor -> Particle relation for the cases where multiple particles match
  vector< pair< int, set<int> > > multimatch;
  // Are there ambiguities in the match?
  bool isAmbiguities = false;
  // The particle daughters that have been matched
  set<int> matches_global;

  // check if the daughters match
  for (int iDaughter_d = 0; iDaughter_d < getNDaughters(); iDaughter_d++) {
    set<int> matches;
    for (int jDaughter_p = 0; jDaughter_p < nDaughters_p; jDaughter_p++) {
      const T* daughter = daughterList[jDaughter_p];
      int iPDGCode_daughter_p = 0;
      if (const auto* part_test = dynamic_cast<const Particle*>(daughter))
        iPDGCode_daughter_p = part_test->getPDGCode();
      else if (const auto* mc_test = dynamic_cast<const MCParticle*>(daughter))
        iPDGCode_daughter_p = mc_test->getPDG();

      if (iDaughter_d == 0 && (this->isIgnoreRadiatedPhotons() or this->isIgnoreGamma() or this->isIgnoreBrems())
          && iPDGCode_daughter_p == Const::photon.getPDGCode())
        matches_global.insert(jDaughter_p);

      int iMatchResult = m_daughters[iDaughter_d].match(daughter, jDaughter_p);
      if (iMatchResult < 0) isAmbiguities = true;
      if (abs(iMatchResult) == 2 && iCC == 1) continue;
      if (abs(iMatchResult) == 1 && iCC == 2) continue;
      if (abs(iMatchResult) == 2 && iCC == 3) continue;
      matches.insert(jDaughter_p);
      matches_global.insert(jDaughter_p);
    }
    if (matches.empty()) return 0;
    if (matches.size() == 1) {
      int jDaughter_p = *(matches.begin());
      singlematch.emplace_back(iDaughter_d, jDaughter_p);
    } else multimatch.emplace_back(iDaughter_d, matches);
  }

  // Now, all daughters of the particles should be matched to at least one DecayDescriptor daughter
  if (!(this->isIgnoreIntermediate() or this->isIgnoreMassive() or this->isIgnoreNeutrino())
      && int(matches_global.size()) != nDaughters_p) return 0;

  // In case that there are DecayDescriptor daughters with multiple matches, try to solve the problem
  // by removing the daughter candidates which are already used in other unambiguous relations.
  // This is done iteratively. We limit the maximum number of attempts to 20 to avoid an infinite loop.
  bool isModified = true;
  for (int iTry = 0; iTry < 20; iTry++) {
    if (int(singlematch.size()) == getNDaughters()) break;
    if (!isModified) break;
    isModified = false;
    for (auto& itMulti : multimatch) {
      for (auto& itSingle : singlematch) {
        // try to remove particle from the multimatch list
        if (itMulti.second.erase(itSingle.second)) {
          B2FATAL("Trying to execute part of the code with known bug, which is not fixed yet! Send email to anze.zupanc@ijs.si with notification that this happens!");
          /*
            This part of the code is commented, because of the following error:
            Iterator 'itMulti' used after element has been erased.

                // if multimatch list contains only one particle candidate, move the entry to the singlematch list
                if (itMulti->second.size() == 1) {
                  int iDaughter_d = itMulti->first;
                  int iDaughter_p = *(itMulti->second.begin());
                  singlematch.push_back(make_pair(iDaughter_d, iDaughter_p));
                  multimatch.erase(itMulti);
                  // call match function again to set the correct daughter
                  if (!isAmbiguities) {
                    const T* daughter = daughterList[iDaughter_p];
                    if (!daughter) continue;
                    m_daughters[iDaughter_d].match(daughter, iDaughter_p);
                  }
                  --itMulti;
                  isModified = true;
                  break;
                }
          */
        }
      }
    }
  }

  if (!multimatch.empty()) isAmbiguities = true;
  if (isAmbiguities) return -iCC;
  else {
    m_iDaughter_p = iDaughter_p;
    return iCC;
  }
  return 0;
}

void DecayDescriptor::resetMatch()
{
  m_iDaughter_p = -1;
  int nDaughters = m_daughters.size();
  for (int iDaughter = 0; iDaughter < nDaughters; iDaughter++) m_daughters[iDaughter].resetMatch();
}

vector<const Particle*> DecayDescriptor::getSelectionParticles(const Particle* particle)
{
  // Create vector for output
  vector<const Particle*> selparticles;
  if (m_mother.isSelected()) {
    int motherPDG = abs(particle->getPDGCode());
    int decayDescriptorMotherPDG = abs(m_mother.getPDGCode());
    if (motherPDG != decayDescriptorMotherPDG)
      B2ERROR("The PDG code of the mother particle (" << motherPDG <<
              ") does not match the PDG code of the DecayDescriptor mother (" << decayDescriptorMotherPDG <<
              ")! Check the order of the decay string is the same you expect in the reconstructed Particles.");
    selparticles.push_back(particle);
  }
  int nDaughters_d = getNDaughters();
  for (int iDaughter_d = 0; iDaughter_d < nDaughters_d; ++iDaughter_d) {
    // retrieve the particle daughter ID from this DecayDescriptor daughter
    int iDaughter_p = m_daughters[iDaughter_d].getMatchedDaughter();
    // If the particle daughter ID is below one, the match function was not called before
    // or the match was ambiguous. In this case try to use the daughter ID of the DecayDescriptor.
    // This corresponds to using the particle order in the decay string.
    if (iDaughter_p < 0) iDaughter_p = iDaughter_d;
    const Particle* daughter = particle->getDaughter(iDaughter_p);
    if (!daughter) {
      B2WARNING("Could not find daughter!");
      continue;
    }
    // check if the daughter has the correct PDG code
    int daughterPDG = abs(daughter->getPDGCode());
    int decayDescriptorDaughterPDG = abs(m_daughters[iDaughter_d].getMother()->getPDGCode());
    if (daughterPDG != decayDescriptorDaughterPDG) {
      B2ERROR("The PDG code of the particle daughter (" << daughterPDG <<
              ") does not match the PDG code of the DecayDescriptor daughter (" << decayDescriptorDaughterPDG <<
              ")! Check the order of the decay string is the same you expect in the reconstructed Particles.");
      break;
    }
    vector<const Particle*> seldaughters = m_daughters[iDaughter_d].getSelectionParticles(daughter);
    selparticles.insert(selparticles.end(), seldaughters.begin(), seldaughters.end());
  }
  return selparticles;
}

bool DecayDescriptor::isSelfConjugated() const
{

  std::vector<int> decay, decaybar;
  for (int i = 0; i < getNDaughters(); ++i) {
    const DecayDescriptorParticle* daughter = getDaughter(i)->getMother();
    int pdg = daughter->getPDGCode();
    decay.push_back(pdg);
    decaybar.push_back(Belle2::EvtPDLUtil::hasAntiParticle(pdg) ? -pdg : pdg);
  }

  std::sort(decay.begin(), decay.end());
  std::sort(decaybar.begin(), decaybar.end());

  return (not Belle2::EvtPDLUtil::hasAntiParticle(getMother()->getPDGCode())) || (decay == decaybar);

}

vector<string> DecayDescriptor::getSelectionNames()
{
  vector<string> strNames;
  if (m_mother.isSelected()) strNames.push_back(m_mother.getNameSimple());
  for (auto& daughter : m_daughters) {
    vector<string> strDaughterNames = daughter.getSelectionNames();
    int nDaughters = strDaughterNames.size();
    for (int iDaughter = 0; iDaughter < nDaughters; iDaughter++) {
      // Checking variable naming scheme from AnalysisConfiguratin
      // For example, effect of possible schemes for PX variable
      // of pi0 from D in decay B->(D->pi0 pi) pi0:
      // default: B_D_pi0_PX
      // semidefault: D_pi0_PX
      // laconic: pi01_PX
      if (AnalysisConfiguration::instance()->getTupleStyle() == "laconic") continue;
      if ((AnalysisConfiguration::instance()->getTupleStyle() == "semilaconic") && (iDaughter == nDaughters)) continue;
      strDaughterNames[iDaughter] = m_mother.getNameSimple() + "_" + strDaughterNames[iDaughter];
    }
    strNames.insert(strNames.end(), strDaughterNames.begin(), strDaughterNames.end());
  }

  // search for multiple occurrence of the same name and then distinguish by attaching a number

  for (auto itName = strNames.begin(); itName != strNames.end(); ++itName) {
    if (count(itName, strNames.end(), *itName) == 1) continue;
    // multiple occurrence found!
    string strNameOld = *itName;
    auto itOccurrence = strNames.begin();
    int iOccurrence = 0;
    while (iOccurrence <= 10) {
      // find next occurrence of the identical particle name defined in DecayDescriptor
      itOccurrence = find(itOccurrence, strNames.end(), strNameOld);
      // stop, if nothing found
      if (itOccurrence == strNames.end()) break;
      // create new particle name by attaching a number
      string strNameNew = strNameOld + std::to_string(iOccurrence);
      // check if the new particle name exists already, if not, then it is OK to use it
      if (count(strNames.begin(), strNames.end(), strNameNew) == 0) {
        *itOccurrence = strNameNew;
        ++itOccurrence;
      }
      iOccurrence++;
    }
    if (iOccurrence == 10) {
      B2ERROR("DecayDescriptor::getSelectionNames - Something is wrong! More than 10x the same name!");
      break;
    }
  }
  return strNames;
}

vector<int> DecayDescriptor::getSelectionPDGCodes()
{
  vector<int> listPDG;
  if (m_mother.isSelected()) listPDG.push_back(m_mother.getPDGCode());
  for (auto& daughter : m_daughters) {
    vector<int> listPDGDaughters = daughter.getSelectionPDGCodes();
    listPDG.insert(listPDG.end(), listPDGDaughters.begin(), listPDGDaughters.end());
  }
  return listPDG;
}


std::vector<std::vector<std::pair<int, std::string>>>  DecayDescriptor::getHierarchyOfSelected()
{
  if (not m_hierarchy.empty()) {
    std::vector<std::vector<std::pair<int, std::string>>> hierarchy = m_hierarchy;
    return hierarchy;
  }
  std::vector<std::pair<int, std::string>> currentPath;
  currentPath.emplace_back(0, m_mother.getNameSimple());
  return getHierarchyOfSelected(currentPath);
}

std::vector<std::vector<std::pair<int, std::string>>>  DecayDescriptor::getHierarchyOfSelected(
  const std::vector<std::pair<int, std::string>>& currentPath)
{
  if (m_mother.isSelected()) m_hierarchy.push_back(currentPath);
  for (std::size_t i = 0; i < m_daughters.size(); i++) {
    std::vector<std::pair<int, std::string>> newPath = currentPath;
    newPath.emplace_back(i, m_daughters[i].getMother()->getNameSimple());
    std::vector<std::vector<std::pair<int, std::string>>> foundPathes = m_daughters[i].getHierarchyOfSelected(newPath);
    for (auto& path : foundPathes) m_hierarchy.push_back(path);
  }
  std::vector<std::vector<std::pair<int, std::string>>> hierarchy = m_hierarchy;
  return hierarchy;
}
