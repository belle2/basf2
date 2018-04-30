/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Anze Zupanc                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLERECOILKINEMATICSTOOL_H
#define NTUPLERECOILKINEMATICSTOOL_H

#include <analysis/NtupleTools/NtupleFlatTool.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>

#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /**
   * NtupleTool to write the kinematics of the recoil system that recoils against given particle.
   */
  class NtupleRecoilKinematicsTool : public NtupleFlatTool {

  private:

    float** m_recoilP3; /**< 3-momentum recoiling against given Particle */
    float* m_recoilP;   /**< magnitude of 3-momentum recoiling against given Particle */
    float* m_recoilE;   /**< energy recoiling against given Particle */
    float* m_recoilM;   /**< invariant mass of the system recoiling against given Particle */
    float* m_recoilMM2; /**< invarian mass squared of the system recoiling against given Particle */

    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();

  public:
    /** Constuctor. */
    NtupleRecoilKinematicsTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}

    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLERECOILKINEMATICSTOOL_H
