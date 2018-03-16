/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christopher Hearty                                       *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEMCGENCMSKINEMATICSTOOL_H
#define NTUPLEMCGENCMSKINEMATICSTOOL_H
#include <boost/function.hpp>
#include <analysis/dataobjects/Particle.h>
#include <framework/dataobjects/EventMetaData.h>
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>

namespace Belle2 {
  /** Tool for NtupleMaker to write out container summaries to flat ntuple. */
  class NtupleMCGenCMSKinematicsTool : public NtupleFlatTool {
  private:
    /** Number of generated MC particles stored */
    int m_nMCGenCMS;
    /** PDG number of each */
    int* m_MCGenCMSPDG;
    /** Index of the mother of this particle in ntuple. -1 means mother is virtual */
    int* m_MCGenCMSMothIndex;
    /** Magnitude of momentum (center of mass frame) */
    float* m_MCGenCMSP;
    /** Theta (center of mass frame) */
    float* m_MCGenCMSTheta;
    /** Phi (center of mass frame) */
    float* m_MCGenCMSPhi;
    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
  public:
    /** Constructor. */
    NtupleMCGenCMSKinematicsTool(TTree* tree, DecayDescriptor& decaydescriptor, const std::string& strOptions) : NtupleFlatTool(tree,
          decaydescriptor, strOptions) {setupTree();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };
} // namepspace Belle2

#endif // NTUPLEMCGENCMSKINEMATICSTOOL_H
