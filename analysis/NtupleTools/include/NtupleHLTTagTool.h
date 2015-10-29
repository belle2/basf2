/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Chunhua LI                                               *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEHLTTAGTOOL_H
#define NTUPLEHLTTAGTOOL_H

#include <analysis/NtupleTools/NtupleFlatTool.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /**
   * NtupleTool to write the trigger results with L1 emulator to a flat ntuple
   */
  class NtupleHLTTagTool : public NtupleFlatTool {

  private:
    /**the total trigger summary*/
    int m_hltsummary;

    /**the sub trigger results*/
    int* m_hltsubtrgsummary;

    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();

  public:
    /** Constuctor. */
    NtupleHLTTagTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}

    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLEROEMULTIPLICITIESTOOL_H
