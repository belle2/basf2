/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Bastian Kronenbitter                                     *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleMCDecayStringTool.h>

#include <analysis/dataobjects/DecayHashMap.h>
#include <framework/datastore/StoreObjPtr.h>

#include <TTree.h>
#include <TBranch.h>
#include <sstream>

using namespace Belle2;

NtupleMCDecayStringTool::NtupleMCDecayStringTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor)
{
  setupTree();

  StoreObjPtr<DecayHashMap>::required();
}

void NtupleMCDecayStringTool::setupTree()
{
  std::vector<std::string> strNames = m_decaydescriptor.getSelectionNames();
  if (strNames.empty()) return;
  m_decayString[0] = '\0';

  std::stringstream ss;
  ss << strNames[0] << "_mcDecayString[" << c_lengthOfDecayString << "]/C";
  m_tree->Branch((strNames[0] + "_mcDecayString").c_str(), m_decayString, ss.str().c_str());
}

void NtupleMCDecayStringTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtupleMCDecayStringTool::eval - no Particle found!");
    return;
  }

  std::vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  if (selparticles.empty())
    return;

  StoreObjPtr<DecayHashMap> dMap;

  std::string decayString = dMap->getDecayString(particle->getExtraInfo(c_ExtraInfoName));

  //fail safe copy of string to cstring
  strncpy(m_decayString, decayString.c_str(), c_lengthOfDecayString - 1);

  if (decayString.length() >= c_lengthOfDecayString) {
    m_decayString[c_lengthOfDecayString - 1] = '\0';
  }
}




