/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Chunhua LI                                               *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLELEECLTOOL_H
#define NTUPLELEECLTOOL_H

#include <analysis/NtupleTools/NtupleFlatTool.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /**
   * NtupleTool to write the ECL information used in L1 Emulator to a flat ntuple
   */
  class NtupleLEECLTool : public NtupleFlatTool {

  private:

    /**the number of ECL Cluster*/
    int m_ICN;

    /**the total ECL energy*/
    float m_Etot;

    /**
    *information of the most energnetic ECL cluster, [E, R, Theta, Phi]
    *E: energy, R: the distance between cluster an IP, Theta: polar angle, Phi: the azimuthal angle
    */
    float* m_Vp4E1;

    /**information of the second most energnetic ECL cluster, [E, R, Theta, Phi]*/

    float* m_Vp4E2;

    /**the angle between the most and the second most energnetic ECL clusters*/
    float m_AngleGG;

    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();

  public:
    /** Constuctor. */
    NtupleLEECLTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}

    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLEROEMULTIPLICITIESTOOL_H
