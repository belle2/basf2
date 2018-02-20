/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald, Phillip Urquijo                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

#include <boost/function.hpp>
#include <analysis/dataobjects/Particle.h>
#include <framework/dataobjects/EventMetaData.h>
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>

namespace Belle2 {
  /** Tool for NtupleMaker to write out container summaries to flat ntuple. */
  class NtupleRecoStatsTool : public NtupleFlatTool {
  private:
    /** N ECL clusters */
    int m_iClusters;
    /** N neutral ECL clusters */
    int m_iNeutralClusters;
    /** N charged ECL clusters */
    int m_iChargedClusters;
    /** N good neutral ECL clusters */
    int m_iGoodNeutralClusters;
    /** neutral ECL energy */
    float m_neutralECLEnergy;
    /** good neutral ECL energy */
    float m_goodNeutralECLEnergy;
    /** charged ECL energy */
    float m_chargedECLEnergy;
    /** N Tracks*/
    int m_iTracks;
    /** N MCParticles*/
    int m_iMCParticles;
    /** N Particles*/
    int m_iParticles;
    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
  public:
    /** Constructor. */
    NtupleRecoStatsTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };
} // namepspace Belle2

