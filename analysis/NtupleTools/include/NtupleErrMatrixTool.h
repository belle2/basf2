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
    /** Error matrix elements */
    float* m_err00;
    float* m_err10;
    float* m_err11;
    float* m_err20;
    float* m_err21;
    float* m_err22;
    float* m_err30;
    float* m_err31;
    float* m_err32;
    float* m_err33;
    float* m_err40;
    float* m_err41;
    float* m_err42;
    float* m_err43;
    float* m_err44;
    float* m_err50;
    float* m_err51;
    float* m_err52;
    float* m_err53;
    float* m_err54;
    float* m_err55;
    float* m_err60;
    float* m_err61;
    float* m_err62;
    float* m_err63;
    float* m_err64;
    float* m_err65;
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
