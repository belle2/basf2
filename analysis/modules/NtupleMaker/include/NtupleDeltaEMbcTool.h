/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald                                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEDELTAEMBCTOOL_H
#define NTUPLEDELTAEMBCTOOL_H
#include <analysis/modules/NtupleMaker/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/modules/NtupleMaker/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

using namespace std;
using namespace Belle2;

namespace Belle2 {

  /** Writes DeltaE and M_bc of B candidate to flat ntuple. */
  class NtupleDeltaEMbcTool : public NtupleFlatTool {
  private:
    /** Delta E. */
    float m_fDeltaE;
    /** beam constraint mass. */
    float m_fMbc;
    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
  public:
    /** Constructor. */
    NtupleDeltaEMbcTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLEDELTAEMBCTOOL_H
