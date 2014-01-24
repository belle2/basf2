/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Anze Zupanc                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEROEMULTIPLICITIESTOOL_H
#define NTUPLEROEMULTIPLICITIESTOOL_H

#include <analysis/NtupleTools/NtupleFlatTool.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>

#include <TTree.h>
#include <string>
#include <utility>

using namespace std;
using namespace Belle2;

namespace Belle2 {

  /**
   * NtupleTool to write the multiplicities of RestOfEvent for given reconstructed Particle
   * to a flat ntuple. The tool writes out the number of remaining tracks (Track), ECL showers,
   * (ECLShower), photons (ECLGamma), and pi0 candidates (ECLPi0), form the RestOfEvent object.
   */
  class NtupleROEMultiplicitiesTool : public NtupleFlatTool {

  private:

    int* m_nTracks;     /**< Number of remaining tracks. */
    int* m_nECLShowers; /**< Number of remaining ECL showers. */
    int* m_nECLGammas;  /**< Number of remaining ECL gammas. */
    int* m_nECLPi0s;    /**< Number of remaining ECL pi0s. */


    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();

  public:
    /** Constuctor. */
    NtupleROEMultiplicitiesTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}

    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLEROEMULTIPLICITIESTOOL_H
