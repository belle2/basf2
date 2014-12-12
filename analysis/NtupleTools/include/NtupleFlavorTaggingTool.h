/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                      *
* Copyright(C) 2014 - Belle II Collaboration                              *
*                                                                         *
* Author: The Belle II Collaboration                                      *
* Contributors: Fernando Abudinen and Moritz Gelb                         *
*                                                                         *
* This software is provided "as is" without any warranty.                 *
**************************************************************************/

#ifndef NTUPLEFLAVORTAGGINGTOOL_H
#define NTUPLEFLAVORTAGGINGTOOL_H
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

using namespace std;
using namespace Belle2;

namespace Belle2 {

  /** Writes the flavour tagging output of the B candidate to flat ntuple. */
  class NtupleFlavorTaggingTool : public NtupleFlatTool {
  private:
    float B0Probability;
    float B0barProbability;
    float qrCombined;
    float qrMC;
    float NTracksInROE;
    float NECLClustersInROE;
    float NKLMClustersInROE;

    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
  public:
    /** Constructor. */
    NtupleFlavorTaggingTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLEFLAVORTAGGINGTOOL_H