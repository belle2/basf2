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

  /** Writes all the Flavor Tag Information to flat ntuple. All members are arrays,
   whose elements correspond to all the categories used in the TMVA algorithm*/
  class NtupleFlavorTagInfoTool : public NtupleFlatTool {
  private:
    /** probability of the selected track to be daugther of the Btag (given by TMVA) */
    float* m_targetP;
    /** probability of the selected track to belong to each category (given by TMVA)*/
    float* m_categoryP;
    /** absolute momentum **/
    float* m_P;
    /** 0 - not from Btag, 1-5 - code of the parent meson  **/
    float* m_isFromB;
    float* m_D0;
    float* m_Z0;
    float* m_MCvertex;

    float m_isFromBGeneral;
    float m_ROEComesFromB;
    float m_goodTracksPurityFT;
    float m_goodBadTracksRatioFT;
    float m_goodTracksPurityROE;
    float m_goodBadTracksRatioROE;
    float m_badTracksROE;
    float m_goodTracksROE;
    float m_badTracksFT;
    float m_goodTracksFT;
    float m_FTandROEGoodTracksRatio;
    /** categories used in the TMVA */
    std::string m_categories[8] = {"Electron", "Muon", "KinLepton", "Kaon", "SlowPion", "FastPion", "Lambda", "MaximumP"};

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
