/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Ami Rostomyan                                            *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEROEGAMMASTOOL_H
#define NTUPLEROEGAMMASTOOL_H

#include <analysis/NtupleTools/NtupleFlatTool.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>

#include <TTree.h>
#include <string>
#include <utility>

// #include <TLorentzVector.h>

namespace Belle2 {

  /**
   * NtupleTool to write the kinematics of photons that belong to the RestOfEvent for given reconstructed Particle to a flat ntuple.
   */
  class NtupleROEGammasTool : public NtupleFlatTool {

  private:
    int* m_nROEGoodGammas; /*< Number of good ECL showers in ROE. */
    float m_fP[20];        /*< Total momentum. */
    float m_fP4[20][4];    /*< Momentum in lab system (px py pz e). */
    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();

  public:
    /** Constuctor. */
    NtupleROEGammasTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}

    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLEROEGAMMASTOOL_H
