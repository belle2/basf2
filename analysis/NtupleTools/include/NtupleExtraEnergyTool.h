/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Anze Zupanc                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEEXTRAENERGYTOOL_H
#define NTUPLEEXTRAENERGYTOOL_H

#include <analysis/NtupleTools/NtupleFlatTool.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>

#include <TTree.h>
#include <string>
#include <utility>

using namespace std;
using namespace Belle2;

namespace Belle2 {

  /**
   * NtupleTool to write the extra energy in the ECL
   */
  class NtupleExtraEnergyTool : public NtupleFlatTool {

  private:

    float* m_extraE;   /**< Energy detected in the ECL that is not associated to the given Particle. */

    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();

  public:
    /** Constuctor. */
    NtupleExtraEnergyTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}

    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLEEXTRAENERGYTOOL_H
