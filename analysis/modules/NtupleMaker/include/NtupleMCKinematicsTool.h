/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald, Phillip Urquijo                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEMCKINEMATICSTOOL_H
#define NTUPLEMCKINEMATICSTOOL_H
#include <analysis/modules/NtupleMaker/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/modules/NtupleMaker/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

using namespace std;
using namespace Belle2;

namespace Belle2 {

  /** NtupleTool to write the kinematics of an MCParticle matched to the reconstructed Particle
  (momentum, energy, mass) to a flat ntuple. */
  class NtupleMCKinematicsTool : public NtupleFlatTool {
  private:
    /** Total momentum. */
    float* m_fP;
    /** Momentum in lab system (x component). */
    float* m_fPx;
    /** Momentum in lab system (y component). */
    float* m_fPy;
    /** Momentum in lab system (z component). */
    float* m_fPz;
    /** Energy in lab system. */
    float* m_fE;
    /** Mass. */
    float* m_fM;
    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
  public:
    /** Constuctor. */
    NtupleMCKinematicsTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLEMCKINEMATICSTOOL_H
