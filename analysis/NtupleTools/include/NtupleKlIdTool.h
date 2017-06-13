/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald,Phillip Urquijo                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEKLIDTOOL_H
#define NTUPLEKLIDTOOL_H
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /** NtupleTool to write the KlId variables to a flat ntuple. */
  class NtupleKlIdTool : public NtupleFlatTool {
  private:
    /** PID. */
    float* m_KlId;
    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
  public:
    /** Constuctor. */
    NtupleKlIdTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLEPIDTOOL_H
