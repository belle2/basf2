/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Phillip Urquijo                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEDALITZTOOL_H
#define NTUPLEDALITZTOOL_H
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /**
   * Writes the Dalitz masses for 3 body decays
   */

  class NtupleDalitzTool : public NtupleFlatTool {
  private:

    /** invariant mass of daughters 1 and 2 */
    float m_m12;
    /** invariant mass of daughters 1 and 3 */
    float m_m13;
    /** invariant mass of daughters 2 and 3 */
    float m_m23;

    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree() override;

  public:

    /** Constructor. */
    NtupleDalitzTool(TTree* tree, DecayDescriptor& decaydescriptor) :  NtupleFlatTool(tree, decaydescriptor)
      , m_m12(0), m_m13(0), m_m23(0)
    {
      setupTree();
    }

    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p) override;

  };

} // namepspace Belle2

#endif // NTUPLEDALITZTOOL_H
