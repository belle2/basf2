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

  /**
   * NtupleTool to write the output of the FlavorTagger for all RestOfEvent B0s to a flat ntuple.
   * The tool writes out the probabilities for B0 (B0Probability) and B0bar (B0barProbability),
   * the combined product dilution*flavor (qrCombined) and the MC flavor (qrMC).
   */
  class NtupleFlavorTaggingTool : public NtupleFlatTool {

  private:

    /** Flag to save the Flavor Tagger Output using the TMVA Output. Default is to use the TMVA Output. */
    bool m_useFANN;

    /**< Direct Output of the Combiner: Probability of being a B0.*/
    float* B0Probability;

    /**< Direct Output of the Combiner: Probability of being a B0bar).*/
    float* B0barProbability;

    /**< qr Output of the Combiner. Output of the FlavorTagger after the complete process*/
    float* qrCombined;

    /**< q Flavor of the B0-Meson related to the RestOfEvent. Only saved if there is MC Info.*/
    float* qrMC;

    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();

  public:
    /** Constructor. */
    NtupleFlavorTaggingTool(TTree* tree, DecayDescriptor& decaydescriptor, std::string strOptions) : NtupleFlatTool(tree,
          decaydescriptor, strOptions)
    {
      m_useFANN = false;
      setupTree();
    }

    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLEFLAVORTAGGINGTOOL_H