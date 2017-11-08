/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald,Phillip Urquijo                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEPIDTOOL_H
#define NTUPLEPIDTOOL_H
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /** NtupleTool to write the kinematics of a Particle
  (momentum, energy, mass) to a flat ntuple. */
  class NtuplePIDTool : public NtupleFlatTool {
  private:
    /** kaon PID. */
    float* m_fPIDk;
    /** pion PID. */
    float* m_fPIDpi;
    /** electron PID. */
    float* m_fPIDe;
    /** muon PID. */
    float* m_fPIDmu;
    /** proton PID. */
    float* m_fPIDp;
    /** deuteron PID. */
    float* m_fPIDd;
    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
    /** delete 'new's */
    void deallocateMemory();
  public:
    /** Constuctor. */
    NtuplePIDTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}

    /** Destructor. */
    ~NtuplePIDTool() {deallocateMemory();}

    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLEPIDTOOL_H
