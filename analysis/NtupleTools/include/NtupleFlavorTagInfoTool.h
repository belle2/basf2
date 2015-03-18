/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Roca                                           *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEFLAVORTAGINFOTOOL_H
#define NTUPLEFLAVORTAGINFOTOOL_H
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /** Writes tag vertex position and error of B candidate to flat ntuple. */
  class NtupleFlavorTagInfoTool : public NtupleFlatTool {
  private:
    /** reconstructed tag vertex x component. */
    float* m_targetP;
    /** reconstructed tag vertex y component. */
    float* m_categoryP;
    float* m_isB;
    /** reconstructed tag vertex z component. */
    std::string m_categories[8] = {"Electron", "Muon", "KinLepton", "Kaon", "SlowPion", "FastPion", "Lambda", "MaximumP*"};

    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
  public:
    /** Constructor. */
    NtupleFlavorTagInfoTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLEFLAVORTAGINFOTOOL_H
