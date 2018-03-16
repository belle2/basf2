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
  /* WORK IN PROGRESS */

  /** Writes all information in the FlavorTagInfo DataObject to a flat ntuple. Some of the elements
   * beneath are arrays, correspoding to the vectors inside the FlavorTagInfo object. That is, one
   * element of the vector per category. For more information, check FlavorTagInfo.h and the
   * FlavorTagger.py script. */
  class NtupleFlavorTagInfoTool : public NtupleFlatTool {
  private:
    /** probability of each track to be a primary track */
    float* m_targetP;
    /** probability of the event to belong to each category */
    float* m_categoryP;
    /** absolute 3 momentum of each track **/
    float* m_P;
    /** code of each track regarding its MC matched mother (legend in FlavorTagInfo.h)  **/
    float* m_isFromB;
    /** impact parameter D0 of each track **/
    float* m_D0;
    /** impact parameter Z0 of each track **/
    float* m_Z0;
    /** resolution of the production point of each track **/
    float* m_prodPointResolutionZ;
    /** purity of tracks coming from Btag in the FlavorTagInfo object **/
    float m_goodTracksPurityFT;
    /** purity of tracks coming from Btag in the RestOfEvent object **/
    float m_goodTracksPurityROE;
    /** number of tracks not coming from Btag in the RestOfEvent object **/
    float m_badTracksROE;
    /** number of tracks not coming from Btag in the RestOfEvent object **/
    float m_goodTracksROE;
    /** number of tracks coming from Btag in the FlavorTagInfo object **/
    float m_badTracksFT;
    /** number of tracks coming from Btag in the FlavorTagInfo object **/
    float m_goodTracksFT;
    /** categories used in the TMVA */
    std::string m_categories[8] = {"Electron", "Muon", "KinLepton", "Kaon", "SlowPion", "FastHadron", "Lambda", "MaximumP"};

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
