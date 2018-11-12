/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Luigi Li Gioi                                            *
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

  /** Writes generated tag vertex posiotion of B candidate to flat ntuple. */
  class NtupleMCTagVertexTool : public NtupleFlatTool {
  private:
    /** generated tag vertex x component. */
    float m_fMCTagVx;
    /** generated tag vertex y component. */
    float m_fMCTagVy;
    /** generated tag vertex z component. */
    float m_fMCTagVz;
    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree() override;
  public:
    /** Constructor. */
    NtupleMCTagVertexTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p) override;
  };

} // namepspace Belle2

