/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Anze Zupanc                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEDELTALOGL_H
#define NTUPLEDELTALOGL_H
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /** NtupleTool to write DeltaLogLikelihoods for allparticle hyothesis*/
  class NtupleDeltaLogLTool : public NtupleFlatTool {
  private:
    /** Delta Log L = L(particle's hypothesis) - L(pion) */
    float* m_fDLogLPion;
    /** Delta Log L = L(particle's hypothesis) - L(kaon) */
    float* m_fDLogLKaon;
    /** Delta Log L = L(particle's hypothesis) - L(proton) */
    float* m_fDLogLProt;
    /** Delta Log L = L(particle's hypothesis) - L(electron) */
    float* m_fDLogLElec;
    /** Delta Log L = L(particle's hypothesis) - L(muon) */
    float* m_fDLogLMuon;

    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();

  public:
    /** Constuctor. */

    NtupleDeltaLogLTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}

    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLEDELTALOGL_H
