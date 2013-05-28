/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald,Phillip Urquijo                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEMCRECONSTRUCTIBLETOOL_H
#define NTUPLEMCRECONSTRUCTIBLETOOL_H
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

using namespace std;
using namespace Belle2;

namespace Belle2 {

  /** NtupleTool to write the MC information of reconstructed Particles
  to a flat ntuple. */
  class NtupleMCReconstructibleTool : public NtupleFlatTool {
  private:
    /** Reconstructible based on particle ID */
    int* m_iReconstructible;
    /** Flags for subdetectors */
    int* m_iSeenInPXD;
    int* m_iSeenInSVD;
    int* m_iSeenInCDC;
    int* m_iSeenInTOP;
    int* m_iLastSeenInECL;
    int* m_iLastSeenInKLM;

    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
  public:
    /** Constuctor. */
    NtupleMCReconstructibleTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLEMCRECONSTRUCTIBLETOOL_H
