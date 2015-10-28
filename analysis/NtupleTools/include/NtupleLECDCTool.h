/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Chunhua LI                                               *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLELECDCTOOL_H
#define NTUPLELECDCTOOL_H

#include <analysis/NtupleTools/NtupleFlatTool.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /**
   * NtupleTool to write the CDC information used in L1 emulator to a flat Ntuple
   */
  class NtupleLECDCTool : public NtupleFlatTool {

  private:

    /**the number of CDC tracks*/
    int m_nTracks;
    /**the number of long CDC tracks*/
    int m_nLongTracks;
    /**the number of CDC tracks matched to ECL cluster*/
    int m_nECLMatchTracks;
    /**the number of CDC tracks matched to KLM cluster*/
    int m_nKLMMatchTracks;

    /**the maximum open angle between the tracks*/
    float m_maxAng;

    /**the polar angle of the track with negative charge*/
    float m_minusTheta;
    /**
      *the information of the track with the largest momentum [P,theta, phi, charge, e ]
      *p:momentum,  theta: polar angle, phi: azimuthal angle, charge, e: the energh of the ECL cluster matched to trk1
      */
    double* m_P1Bhabha;

    /**the information of the track with the second largest momentum [P,theta, phi, harge, e ]*/
    double* m_P2Bhabha;

    /**the P1 in CMS*/
    float  m_P1CMSBhabha;

    /**the P2 in CMS*/
    float  m_P2CMSBhabha;

    /**the visible Pz*/
    float m_VisiblePz;

    /**the visible energy*/
    float m_VisibleEnergy;

    /**the P1+P2 in CMS*/
    float m_P12CMS;

    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();

  public:
    /** Constuctor. */
    NtupleLECDCTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}

    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLEROEMULTIPLICITIESTOOL_H
