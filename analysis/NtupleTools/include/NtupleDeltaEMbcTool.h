/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald                                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /** Writes DeltaE and M_bc of B candidate to flat ntuple. */
  class NtupleDeltaEMbcTool : public NtupleFlatTool {
  private:
    /** Delta E. */
    float m_fDeltaE;
    /** beam constraint mass. */
    float m_fMbc;
    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree() override;
  public:
    /** Constructor. */
    NtupleDeltaEMbcTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p) override;
  };

} // namepspace Belle2

