/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Sam Cunliffe                                             *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEPARENTRESTKINEMATICSTOOL_H
#define NTUPLEPARENTRESTKINEMATICSTOOL_H

#include <analysis/NtupleTools/NtupleFlatTool.h>

#include <analysis/dataobjects/Particle.h>

#include <analysis/DecayDescriptor/DecayDescriptor.h>

#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /**
   * NtupleTool to write the kinematics of a Particle
   * in it's parent's (i.e. the head of the particle collection) rest frame
   */

  class NtupleParentRestKinematicsTool : public NtupleFlatTool {

  private:

    /** Parent rest frame momentum magnitude */
    float* m_fP;

    /** Parent rest frame 4-momentum vector (px py pz e) */
    float** m_fP4;

    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();

  public:

    /** Constuctor. */
    NtupleParentRestKinematicsTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}

    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLEPARENTRESTKINEMATICSTOOL_H
