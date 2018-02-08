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
   * - E1E9   : ratio of energies in central cell and 3x3 cells
   * - E9E21  : ratio of energies in inner 3x3 and 5x5-corner cells
   * - nHits  : number of hits associated to this cluster
   * - trackM : 1/0 if charged track is/is not Matched to this cluster
   */
  class NtupleClusterTool : public NtupleFlatTool {
  private:

    /** uncorrected energy */
    float* m_uncorrE;

    /** energy of the most energetic cluster */
    float* m_highestE;

    /** timing */
    float* m_timing;

    /** polar angle */
    float* m_theta;

    /** azimuthal angle */
    float* m_phi;

    /** distance */
    float* m_distance;

    /** detection region in the ECL [1 - forward, 2 - barrel, 3 - backward] */
    int* m_region;

    /**  ratio of energies in central cell and 3x3 cells */
    float* m_e1e9;

    /**  cluster hadron scintillation component intensity*/
    float* m_ClusterHadronIntensity;

    /**  number of cluster digits with significant (>3 MeV) hadron component light output*/
    int* m_NumberofHadronDigits;

    /**  ratio of energies in inner 3x3 and 5x5-corner cells */
    float* m_e9e21;

    /** number of hits associated to this cluster */
    int* m_nHits;

    /**  Status of pulse shape discrimination variables. 0 = no PSD  information (no waveforms), 1 = PSD cluster has PSD information */
    int* m_ClusterPSD;

    /** 1/0 if charged track is/is not Matched to this cluster */
    int* m_trackM;

    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();

    /** Delete the 'new's */
    void deallocateMemory();

  public:

    /** Constuctor. */
    NtupleClusterTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}

    /** Destructor. */
    ~NtupleClusterTool() {deallocateMemory();}

    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLECLUSTERTOOL_H
