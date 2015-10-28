/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Chunhua LI                                               *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLELEKLMTOOL_H
#define NTUPLELEKLMTOOL_H

#include <analysis/NtupleTools/NtupleFlatTool.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /**
   * NtupleTool to write the information of KLM used in L1 Emulator to a flat ntuple
   */
  class NtupleLEKLMTool : public NtupleFlatTool {

  private:
    /**the max angle between KLM clusters*/
    float m_maxAngMM;
    /**the max angle between CDC tracks and KLM clusters*/
    float m_maxAngTM;
    /**the number of KLM clusters*/
    int m_nKLMClusters;
    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
    /**the largest layers of KLM clusters*/
    int m_Layer1;
    /**the second largest layers of KLM clusters*/
    int m_Layer2;


  public:
    /** Constuctor. */
    NtupleLEKLMTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}

    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLEROEMULTIPLICITIESTOOL_H
