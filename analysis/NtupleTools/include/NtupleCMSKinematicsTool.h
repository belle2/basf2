/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Anze Zupanc                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLECMSKINEMATICSTOOL_H
#define NTUPLECMSKINEMATICSTOOL_H

#include <analysis/NtupleTools/NtupleFlatTool.h>

#include <analysis/dataobjects/Particle.h>

#include <analysis/DecayDescriptor/DecayDescriptor.h>

#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /**
   * NtupleTool to write the kinematics of a Particle
   * in the CMS frame to a flat ntuple.
   */

  class NtupleCMSKinematicsTool : public NtupleFlatTool {

  private:
    /** number of decay products */
    int m_nDecayProducts;

    /** CMS momentum magnitude */
    float* m_fP;

    /** CMS 4-momentum vector (px py pz e) */
    float** m_fP4;

    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();

    /** Delete 'new's */
    void deallocateMemory();

  public:

    /** Constuctor. */
    NtupleCMSKinematicsTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}

    /** Destructor. */
    ~NtupleCMSKinematicsTool() {deallocateMemory();}

    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLECMSKINEMATICSTOOL_H
