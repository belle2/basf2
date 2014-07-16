/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald, Luigi Li Gioi                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEKINEMATICSTOOL_H
#define NTUPLEKINEMATICSTOOL_H
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /** NtupleTool to write the kinematics of a Particle
  (momentum, energy, mass) to a flat ntuple. */
  class NtupleKinematicsTool : public NtupleFlatTool {
  private:
    /** Charge. */
    Char_t* m_iQ;
    /** Total momentum. */
    float* m_fP;
    /** Momentum in lab system (px py pz e). */
    float** m_fP4;
    /** Momentum Error in lab system (px py pz e). */
    float** m_fEP4;
    /** Mass. */
    float* m_fM;
    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
  public:
    /** Constuctor. */
    NtupleKinematicsTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLEKINEMATICSTOOL_H
