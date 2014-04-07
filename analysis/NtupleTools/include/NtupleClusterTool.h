/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Anze Zupanc                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLECLUSTERTOOL_H
#define NTUPLECLUSTERTOOL_H
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /**
   * NtupleTool to write out ECL cluster quanties for a given Particle
   * - region : detection region in the ECL [1 - forward, 2 - barrel, 3 - backward]
   * - E9E25  : ratio of energies in inner 3x3 and 5x5 cells
   * - nHits  : number of hits associated to this cluster
   * - trackM : 1/0 if charged track is/is not Matched to this cluster
   */
  class NtupleClusterTool : public NtupleFlatTool {
  private:

    /** detection region in the ECL [1 - forward, 2 - barrel, 3 - backward] */
    int* m_region;

    /**  ratio of energies in inner 3x3 and 5x5 cells */
    float* m_e9e25;

    /** number of hits associated to this cluster */
    int* m_nHits;

    /** 1/0 if charged track is/is not Matched to this cluster */
    int* m_trackM;

    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();

  public:

    /** Constuctor. */
    NtupleClusterTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}

    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLECLUSTERTOOL_H
