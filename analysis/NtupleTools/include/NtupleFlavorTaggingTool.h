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

namespace Belle2 {

  /**
   * NtupleTool to write the output of the FlavorTagger for all RestOfEvent B0s to a flat ntuple.
   * The tool writes out the probabilities for B0 (B0Probability) and B0bar (B0barProbability),
   * the combined product dilution*flavor (qrCombined) and the MC flavor (qrMC).
   */
  class NtupleFlavorTaggingTool : public NtupleFlatTool {

  private:


    /** Flag to save the Flavor Tagger Output using the FANN Output. This is the Default */
    bool m_useFBDT;

    /** qr Output of the Combiner. Output of the FlavorTagger using the FANN MLP method as combiner after the complete process*/
    float* qrCombinedFBDT;

    /** Flag to save the Flavor Tagger Output using the FANN Output.  */
    bool m_useFANN;

    /** qr Output of the Combiner. Output of the FlavorTagger using the FANN MLP method as combiner after the complete process*/
    float* qrCombinedFANN;

    /** q Flavor of the B0-Meson related to the RestOfEvent. Only saved if there is MC Info.*/
    float* qrMC;

    /** Flag to save the individual output of each category.  */
    bool m_saveCategories;

    /** map containing the category name and the corresponding qr Output, i.e. the Combiner input value. They could be used for independent tags.*/
    std::map<std::string, float*> m_qpCategories;

    /** map containing the category name and the corresponding MC Variable isRightTarget. They could be used to evaluate independent tags.*/
    std::map<std::string, float*> m_hasTrueTargets;

    /** map containing the category name and the corresponding MC Variable isRightCategory. They could be used to evaluate independent tags.*/
    std::map<std::string, float*> m_isTrueCategories;

    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();

  public:
    /** Constructor. */
    NtupleFlavorTaggingTool(TTree* tree, DecayDescriptor& decaydescriptor, const std::string& strOptions) : NtupleFlatTool(tree,
          decaydescriptor, strOptions)
    {
      m_useFANN = false;
      m_useFBDT = false;
      m_saveCategories = false;
      setupTree();
    }

    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLEFLAVORTAGGINGTOOL_H
