/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Phillip Urquijo                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLE4BODYTOOL_H
#define NTUPLE4BODYTOOL_H
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /**
   * Writes the decay angles for S to VV in 4 body decays
   */

  class NtupleVVAnglesTool : public NtupleFlatTool {
  private:

    /** helicity angles for A and B daughters in B-> A + B, where A and B decay 2 body */
    float m_helA;
    float m_helB;

    /** planar angle between the two decay planes */
    float m_chi;

    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();

  public:

    /** Constructor. */
    NtupleVVAnglesTool(TTree* tree, DecayDescriptor& decaydescriptor) :  NtupleFlatTool(tree, decaydescriptor)
      , m_helA(0), m_helB(0), m_chi(0) {
      setupTree();
    }

    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);

  };

} // namepspace Belle2

#endif // NTUPLE4BODYTOOL_H
