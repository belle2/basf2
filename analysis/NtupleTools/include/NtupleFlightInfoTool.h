/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Giulia Casarosa                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEFLIGHTINFOTOOL_H
#define NTUPLEFLIGHTINFOTOOL_H
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <TMatrixFSym.h>
#include <string>
#include <utility>

namespace Belle2 {

  /** Writes flight information of decayed particle to flat ntuple. */
  class NtupleFlightInfoTool : public NtupleFlatTool {
  private:

    /** Flight Distance (projected on along the momentum direction)*/
    float m_fD;
    /** Flight Distance Error*/
    float m_fDE;
    /** Flight Time (projected on along the momentum direction)*/
    float m_fT;
    /** Flight Time Error*/
    float m_fTE;
    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
  public:
    /** Constructor. */
    NtupleFlightInfoTool(TTree* tree, DecayDescriptor& decaydescriptor) :  NtupleFlatTool(tree, decaydescriptor)
      , m_fD(0)
      , m_fDE(0)
      , m_fT(0)
      , m_fTE(0) {
      setupTree();
    }
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);

    void evalFlightTime(const Particle* mother, const Particle* daughter);

    void evalFlightDistance(const Particle* mother, const Particle* daughter);
  };

} // namepspace Belle2

#endif // NTUPLEFLIGHTINFOTOOL_H
