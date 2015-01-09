/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Phillip Urquijo                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLESLTOOL_H
#define NTUPLESLTOOL_H
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /**
   * Writes out the lepton helicity angle and q2 of a semileptonic decay
   */

  class NtupleSLTool : public NtupleFlatTool {
  private:

    /** helicity angle in a 2 body decay */
    float m_helA;

    /** lepton momentum transfer */
    float m_q2;


    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();

  public:

    /** Constructor. */
    NtupleSLTool(TTree* tree, DecayDescriptor& decaydescriptor) :  NtupleFlatTool(tree, decaydescriptor)
      , m_helA(0), m_q2(0) {
      setupTree();
    }

    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);

  };

} // namepspace Belle2

#endif // NTUPLESLTOOL_H
