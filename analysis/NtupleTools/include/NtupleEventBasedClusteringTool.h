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
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>

namespace Belle2 {
  /** NtupleTool to write event based clustering quantities to a flat ntuple */
  class NtupleEventBasedClusteringTool : public NtupleFlatTool {
  private:
    /** number of single crystal deposits that are out of time */
    int m_nECLCrystalDepositsOutOfTime;
    /** number of single crystal deposits that are out of time in the FWD endcap */
    int m_nECLCrystalDepositsOutOfTimeFWD;
    /** number of single crystal deposits that are out of time in the barrel */
    int m_nECLCrystalDepositsOutOfTimeBarrel;
    /** number of single crystal deposits that are out of time in the BWD endcap */
    int m_nECLCrystalDepositsOutOfTimeBWD;
    /** number of showers in the ECL that do not become showers */
    int m_nRejectedECLShowers;
    /** number of showers in the ECL that do not become showers, FWD endcap */
    int m_nRejectedECLShowersFWD;
    /** number of showers in the ECL that do not become showers, barrel */
    int m_nRejectedECLShowersBarrel;
    /** number of showers in the ECL that do not become showers, BW endcap */
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
