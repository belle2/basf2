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

    /**the total ECL of C1 and C2*/
    float m_E12;

    /**the CMS values of E1, E2. E12, ENeutral*/
    float* m_ECMS;
    /**
    *information of the most energnetic ECL cluster, [E, R, Theta, Phi]
    *E: energy, R: the distance between cluster an IP, Theta: polar angle, Phi: the azimuthal angle
    */
    float* m_Vp4E1;

    /**information of the second most energnetic ECL cluster, [E, R, Theta, Phi]*/
    float* m_Vp4E2;

    /**information of the most energnetic neutral cluster, [E, R, Theta, Phi]*/
    float* m_Vp4Neutral;


    /**the angle between the most and the second most energnetic ECL clusters*/
    float m_AngleGG;

    /**the max angle between the largest neutral cluster and T1 (T2) */
    float m_AngleGT;

    /**the angle between C1 (C2) and T1 (T2)*/
    float* m_AngCT;

//    float m_EC12CMS;

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
