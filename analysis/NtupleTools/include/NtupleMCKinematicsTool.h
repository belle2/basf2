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
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /** NtupleTool to write the kinematics of an MCParticle matched to the reconstructed Particle
  (momentum, energy, mass) to a flat ntuple. */
  class NtupleMCKinematicsTool : public NtupleFlatTool {
  private:
    /** number of decay products */
    int m_nDecayProducts;
    /** Total momentum. */
    float* m_fTruthP;
    /** 4-Momentum in lab system. */
    float** m_fTruthP4;
    /** Mass. */
    float* m_fTruthM;
    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
    /** delete all 'new's */
    void deallocateMemory();
  public:
    /** Constuctor. */
    NtupleMCKinematicsTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}
    /** Destructor */
    ~NtupleMCKinematicsTool() {deallocateMemory();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLEMCKINEMATICSTOOL_H
