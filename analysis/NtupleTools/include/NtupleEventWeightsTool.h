/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Torben Ferber                                            *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEEVENTWEIGHTSTOOL_H
#define NTUPLEEVENTWEIGHTSTOOL_H
#include <analysis/dataobjects/Particle.h>
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>

namespace Belle2 {
  /** Tool for NtupleMaker to write weight(s) to flat ntuple. */
  class NtupleEventWeightsTool : public NtupleFlatTool {
  private:
    /** generated event weight */
    float m_fWeight;
    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree() override;
  public:
    /** Constructor. */
    NtupleEventWeightsTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p) override;
  };
} // namepspace Belle2

#endif // NTUPLEEVENTWEIGHTSTOOL_H
