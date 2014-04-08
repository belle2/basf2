/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Anze Zupanc                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEMOMENTUMUNCERTAINTYTOOL_H
#define NTUPLEMOMENTUMUNCERTAINTYTOOL_H
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /** NtupleTool to write the kinematics of a Particle
  (momentum, energy, mass) to a flat ntuple. */
  class NtupleMomentumUncertaintyTool : public NtupleFlatTool {
  private:
    /** Uncertainty of the momentum in the lab system (sigma_px sigma_py sigma_pz sigma_e). */
    float** m_fErrP4;
    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
  public:
    /** Constuctor. */
    NtupleMomentumUncertaintyTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLEMOMENTUMUNCERTAINTYTOOL_H
