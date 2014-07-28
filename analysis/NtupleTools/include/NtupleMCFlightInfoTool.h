/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Giulia Casarosa                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEMCFLIGHTINFOTOOL_H
#define NTUPLEMCFLIGHTINFOTOOL_H
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <mdst/dataobjects/MCParticle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /** Writes flight information of decayed particle to flat ntuple. */
  class NtupleMCFlightInfoTool : public NtupleFlatTool {
  private:

    /** True Flight Distance (projected on along the momentum direction)*/
    float m_fD;
    /** True Flight Time (projected on along the momentum direction)*/
    float m_fT;
    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
  public:

    /** Constructor. */
    NtupleMCFlightInfoTool(TTree* tree, DecayDescriptor& decaydescriptor) :  NtupleFlatTool(tree, decaydescriptor)
      , m_fD(0)
      , m_fT(0) {
      setupTree();
    }
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);

  };

} // namepspace Belle2

#endif // NTUPLEMCFLIGHTINFOTOOL_H
