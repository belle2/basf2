/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Anze Zupanc                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEMASSBEFOREFITTOOL_H
#define NTUPLEMASSBEFOREFITTOOL_H
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /**
   * NtupleTool to write out invariant mass of the particle unaffected by any kinematic fit.
   * Mbf      - invariant mass before the kinematic fit
   * Merr     - uncertainty of the invariant mass before the kinematic fit
   */
  class NtupleMassBeforeFitTool : public NtupleFlatTool {
  private:

    /** invariant mass before the kinematic fit */
    float* m_invM;

    /** uncertainty of the invariant mass before the kinematic fit */
    float* m_invMerr;

    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();

    /** Delete 'new's */
    void deallocateMemory();

  public:
    /** Constuctor. */
    NtupleMassBeforeFitTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}

    /** Destructor */
    ~NtupleMassBeforeFitTool() {deallocateMemory();}

    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLEMASSBEFOREFITTOOL_H
