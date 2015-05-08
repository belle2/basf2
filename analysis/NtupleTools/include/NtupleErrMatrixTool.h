/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald, Luigi Li Gioi                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEERRMATRIXTOOL_H
#define NTUPLEERRMATRIXTOOL_H
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TMatrixFSym.h>
#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /** NtupleTool to write the kinematics of a Particle
  (momentum, energy) to a flat ntuple. */
  class NtupleErrMatrixTool : public NtupleFlatTool {
  private:

    /** 7x7 MomentumVertex Error matrix element: E */

    float* m_err00;

    /** 7x7 MomentumVertex Error matrix element: (Px, E) */

    float* m_err10;

    /** 7x7 MomentumVertex Error matrix element: Px */

    float* m_err11;

    /** 7x7 MomentumVertex Error matrix element: (Py, E) */

    float* m_err20;

    /** 7x7 MomentumVertex Error matrix element: (Py, Px) */

    float* m_err21;

    /** 7x7 MomentumVertex Error matrix element: Py */

    float* m_err22;

    /** 7x7 MomentumVertex Error matrix element: (Pz, E) */

    float* m_err30;

    /** 7x7 MomentumVertex Error matrix element: (Pz, Px) */

    float* m_err31;

    /** 7x7 MomentumVertex Error matrix element: (Pz, Py) */

    float* m_err32;

    /** 7x7 MomentumVertex Error matrix element: Pz */

    float* m_err33;

    /** 7x7 MomentumVertex Error matrix element: (X, E) */

    float* m_err40;

    /** 7x7 MomentumVertex Error matrix element: (X, Px) */

    float* m_err41;

    /** 7x7 MomentumVertex Error matrix element: (X, Py) */

    float* m_err42;

    /** 7x7 MomentumVertex Error matrix element: (X, Pz) */

    float* m_err43;

    /** 7x7 MomentumVertex Error matrix element: X */

    float* m_err44;

    /** 7x7 MomentumVertex Error matrix element: (Y, E) */

    float* m_err50;

    /** 7x7 MomentumVertex Error matrix element: (Y, Px) */

    float* m_err51;

    /** 7x7 MomentumVertex Error matrix element: (Y, Py) */

    float* m_err52;

    /** 7x7 MomentumVertex Error matrix element: (Y, Pz) */

    float* m_err53;

    /** 7x7 MomentumVertex Error matrix element: (Y, X) */

    float* m_err54;

    /** 7x7 MomentumVertex Error matrix element: Y */

    float* m_err55;

    /** 7x7 MomentumVertex Error matrix element: (Z, E) */

    float* m_err60;

    /** 7x7 MomentumVertex Error matrix element: (Z, Px) */

    float* m_err61;

    /** 7x7 MomentumVertex Error matrix element: (Z, Py) */

    float* m_err62;

    /** 7x7 MomentumVertex Error matrix element: (Z, Px) */

    float* m_err63;

    /** 7x7 MomentumVertex Error matrix element: (Z, X) */

    float* m_err64;

    /** 7x7 MomentumVertex Error matrix element: (Z, Y) */

    float* m_err65;

    /** 7x7 MomentumVertex Error matrix element: Z */

    float* m_err66;

    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
  public:
    /** Constuctor. */
    NtupleErrMatrixTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLEERRMATRIXTOOL_H
