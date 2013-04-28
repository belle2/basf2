/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald,Phillip Urquijo                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEMCTRUTHTOOL_H
#define NTUPLEMCTRUTHTOOL_H
#include <analysis/modules/NtupleMaker/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/modules/NtupleMaker/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

using namespace std;
using namespace Belle2;

namespace Belle2 {

  /** NtupleTool to write the MC information of reconstructed Particles
  to a flat ntuple. */
  class NtupleMCTruthTool : public NtupleFlatTool {
  private:
    /** Truth match ID. */
    int* m_iTruthID;
    /** Check that the Truth match ID is the same as the Reco PDG ID. */
    int* m_iTruthIDMatch;
    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
  public:
    /** Constuctor. */
    NtupleMCTruthTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLEMCTRUTHTOOL_H
