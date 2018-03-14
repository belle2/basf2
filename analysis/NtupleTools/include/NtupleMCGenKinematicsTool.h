/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christopher Hearty                                       *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEMCGENKINEMATICSTOOL_H
#define NTUPLEMCGENKINEMATICSTOOL_H
#include <boost/function.hpp>
#include <analysis/dataobjects/Particle.h>
#include <framework/dataobjects/EventMetaData.h>
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>

namespace Belle2 {
  /** Tool for NtupleMaker to write out container summaries to flat ntuple. */
  class NtupleMCGenKinematicsTool : public NtupleFlatTool {
  private:
    int m_nMCGen;          /**< Number of generated MC particles stored */
    int* m_MCGenPDG;       /**< PDG number of each */
    int* m_MCGenMothIndex; /**< Index of the mother of this particle in ntuple. -1 means mother is virtual */
    float* m_MCGenP;       /**< the magnitude of the momentum in the laboratory frame */
    float* m_MCGenTheta;   /**< theta in the laboratory frame */
    float* m_MCGenPhi;     /**< phi in the laboratory frame */
    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
  public:
    /** Constructor. */
    NtupleMCGenKinematicsTool(TTree* tree, DecayDescriptor& decaydescriptor, const std::string& strOptions) : NtupleFlatTool(tree,
          decaydescriptor, strOptions) {setupTree();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };
} // namepspace Belle2

#endif // NTUPLEMCGENKINEMATICSTOOL_H
