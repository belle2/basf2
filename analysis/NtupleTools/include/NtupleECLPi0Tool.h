/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Anze Zupanc                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEECLPI0TOOL_H
#define NTUPLEECLPI0TOOL_H
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /**
   * NtupleTool to write out ECLPi0 quanties for a given Particle
   * invM     - invariant mass before the mass constrained fit
   * pValue   - pValue of the mass constrained fit
   * cosTheta - angle between the pi0 thrust vector and the direction of one of the photons in the  pi0 rest frame
   */
  class NtupleECLPi0Tool : public NtupleFlatTool {
  private:

    /** invariant mass before the mass constrained fit */
    float* m_invM;

    /** pValue of the mass constrained fit */
    float* m_pValue;

    /** angle between the pi0 thrust vector and the direction of one of the photons in the  pi0 rest frame */
    float* m_cosTheta;

    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();

  public:
    /** Constuctor. */
    NtupleECLPi0Tool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}

    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLEECLPI0TOOL_H
