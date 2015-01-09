/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Phillip Urquijo                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLE2BODYTOOL_H
#define NTUPLE2BODYTOOL_H
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /**
   * Writes the decay angle of a 2 body decay
   */

  class NtupleHelicityTool : public NtupleFlatTool {
  private:

    /** helicity angle in a 2 body decay */
    float m_helA;


    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();

  public:

    /** Constructor. */
    NtupleHelicityTool(TTree* tree, DecayDescriptor& decaydescriptor) :  NtupleFlatTool(tree, decaydescriptor)
      , m_helA(0) {
      setupTree();
    }

    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);

  };

} // namepspace Belle2

#endif // NTUPLEHELICITYTOOL_H
