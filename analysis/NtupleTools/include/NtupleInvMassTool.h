/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Anze Zupanc                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEINVMASSTOOL_H
#define NTUPLEINVMASSTOOL_H
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /**
   * NtupleTool to write out invariant mass of the particle. Invariant mass unaffected by any kinematic fit can be written out instead with the "BeforeFit" Ntuple tool option
   * M      - invariant mass
   * ErrM   - uncertainty of the invariant mass
   * SigM   - signed significance of the deviation from the nominal mass [(mass - NOMINAL_MASS)/ErrM]
   */

  class NtupleInvMassTool : public NtupleFlatTool {
  private:

    /** invariant mass before the kinematic fit */
    float* m_invM;

    /** uncertainty of the invariant mass before the kinematic fit */
    float* m_invMerr;

    /**  signed significance of the deviation from the nominal mass */
    float* m_invMsig;

    /** flag to save mass before kinematic fit */
    bool m_useMassBeforeFit;

    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();

    /** Delete 'new's */
    void deallocateMemory();


  public:
    /** Constuctor. */
    NtupleInvMassTool(TTree* tree, DecayDescriptor& decaydescriptor, const std::string& strOptions) : NtupleFlatTool(tree,
          decaydescriptor, strOptions)
    {
      m_useMassBeforeFit = false;
      setupTree();
    }

    /** Destructor. */
    ~NtupleInvMassTool() {deallocateMemory();}

    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLEINVMASSTOOL_H
