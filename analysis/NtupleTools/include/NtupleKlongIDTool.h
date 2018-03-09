/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2017 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Jo-Frederik Krohn                                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /** NtupleTool to write the KlongID variables to a flat ntuple. */
  class NtupleKlongIDTool : public NtupleFlatTool {
  private:

    /** the Kl pid */
    float* m_klongID     ;
    /** is Forward EKLM. */
    float* m_isFEKLM  ;
    /** is Backweard EKLM. */
    float* m_isBEKLM  ;
    /** is BKLM. */
    float* m_isBKLM   ;
    /** timeing of KLMcluster. */
    float* m_Time     ;
    /** index of innermost layer. */
    float* m_innerMost;
    /** number of layers. */
    float* m_NLayer   ;
    /** trackFlag. */
    float* m_trackFlag;
    /** ECLFlag. */
    float* m_ECLFlag  ;
    /** energy. */
    float* m_Energy   ;




    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
  public:
    /** Constuctor. */
    NtupleKlongIDTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2
