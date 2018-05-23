/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2018 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Sam Cunliffe, Alon Hershenhorn                           *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once
#include <analysis/NtupleTools/NtupleFlatTool.h>

namespace Belle2 {
  /** NtupleTool to write event based clustering quantities to a flat ntuple */
  class NtupleEventBasedClusteringTool : public NtupleFlatTool {
  private:
    /** number of crystals that are out of time */
    int m_nECLOutOfTimeCrystals;
    /** number of crystals that are out of time in the FWD endcap */
    int m_nECLOutOfTimeCrystalsFWD;
    /** number of crystals that are out of time in the barrel */
    int m_nECLOutOfTimeCrystalsBarrel;
    /** number of crystals that are out of time in the BWD endcap */
    int m_nECLOutOfTimeCrystalsBWD;
    /** number of showers in the ECL that do not become clusters */
    int m_nRejectedECLShowers;
    /** number of showers in the ECL that do not become clusters, FWD endcap */
    int m_nRejectedECLShowersFWD;
    /** number of showers in the ECL that do not become clusters, barrel */
    int m_nRejectedECLShowersBarrel;
    /** number of showers in the ECL that do not become clusters, BW endcap */
    int m_nRejectedECLShowersBWD;
    /** declare branches */
    void setupTree();
  public:
    /** constuctor */
    NtupleEventBasedClusteringTool(TTree* tree, DecayDescriptor& decaydescriptor)
      : NtupleFlatTool(tree, decaydescriptor) {setupTree();}
    /** evaluate the variables */
    void eval(const Particle* p);
  };
} // namepspace Belle2
