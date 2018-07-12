/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Luigi Li Gioi                                            *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLETAGVERTEXTOOL_H
#define NTUPLETAGVERTEXTOOL_H
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /** Writes tag vertex position and error of B candidate to flat ntuple. */
  class NtupleTagVertexTool : public NtupleFlatTool {
  private:
    /** reconstructed tag vertex x component. */
    float m_fTagVx;
    /** reconstructed tag vertex y component. */
    float m_fTagVy;
    /** reconstructed tag vertex z component. */
    float m_fTagVz;
    /** error of the reconstructed tag vertex x component. */
    float m_fTagVex;
    /** error of the reconstructed tag vertex y component. */
    float m_fTagVey;
    /** error of the reconstructed tag vertex z component. */
    float m_fTagVez;
    /** Tag vertex P value. */
    float m_TagVPvalue;
    /** Tag vertex fit Type. */
    int m_TagVType;
    /** Num of tracks used */
    int m_TagVNTracks;
    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
  public:
    /** Constructor. */
    NtupleTagVertexTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLETAGVERTEXTOOL_H
