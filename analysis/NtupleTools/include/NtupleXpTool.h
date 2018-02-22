/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hikari Hirata                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef NTUPLEXPTOOL_H
#define NTUPLEXPTOOL_H
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /**NTupleTool to write the hadron-scaled momentum xp*/
  class NtupleXpTool : public NtupleFlatTool {
  private:

    /** number of decay products */
    int m_nDecayProducts;
    /** xp */
    float* m_fxp;
    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
    /** delete all 'new's */
    void deallocateMemory();

  public:

    /** Constructor. */
    NtupleXpTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}
    /** Destructor */
    ~NtupleXpTool() {deallocateMemory();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namespace Belle2

#endif // NTUPLEXPTOOL_H
