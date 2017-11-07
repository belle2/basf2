/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2017 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Matt Barrett                                             *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleMCDecayStringTool.h>
#include <analysis/utility/MCMatching.h>
#include <mdst/dataobjects/MCParticle.h>

#include <analysis/dataobjects/StringWrapper.h>

#include <TBranch.h>

using namespace Belle2;

void NtupleMCDecayStringTool::setupTree()
{
  std::vector<std::string> strNames = m_decaydescriptor.getSelectionNames();
  if (strNames.empty()) {return;}

  m_mcDecayString = new std::string("Default string.");

  m_tree->Branch((strNames[0] + "_mcDecayString").c_str(), &m_mcDecayString);

}

void NtupleMCDecayStringTool::eval(const Particle* particle)
{

  if (!particle) {
    B2ERROR("NtupleMCDecayStringTool::eval(particle) - particle not found/usable!");
    return;
  }

  std::vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  if (selparticles.empty()) {return;}

  const StringWrapper* stringWrapper = selparticles[0]->getRelated<StringWrapper>();
  if (!stringWrapper) {
    B2ERROR("NtupleMCDecayStringTool::eval - no StringWrapper to hold decay string found! Did you forget the module (ParticleMCDecayStringModule) that builds it?");
    return;
  }

  std::string decayString = stringWrapper->getString();
  m_mcDecayString->assign(decayString);

}
