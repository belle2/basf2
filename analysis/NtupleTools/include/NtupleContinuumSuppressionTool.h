/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Pablo Goldenzweig                                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLECONTINUUMSUPPRESSIONTOOL_H
#define NTUPLECONTINUUMSUPPRESSIONTOOL_H
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

using namespace std;
using namespace Belle2;

namespace Belle2 {

  /** Writes DeltaE and M_bc of B candidate to flat ntuple. */
  class NtupleContinuumSuppressionTool : public NtupleFlatTool {
  private:
    float m_fThrustB; /** magnitude of B thrust axis */
    float m_fThrustO; /** magnitude of ROE thrust axis */

    float m_fCosTBTO; /** cosine of the angle between the thrust axis of the B and the thrust axis of the ROE */
    float m_fCosTBz;  /** cosine of the angle between the thrust axis of the B and the z-axis */

    // For final state = 0
    float m_k0mm2;    /** missing mass squared */
    float m_k0et;     /** transverse energy */
    float m_k0hso00;  /** Hso(0,0) */
    float m_k0hso01;  /** Hso(0,1) */
    float m_k0hso02;  /** Hso(0,2) */
    float m_k0hso03;  /** Hso(0,3) */
    float m_k0hso04;  /** Hso(0,4) */
    float m_k0hso10;  /** Hso(1,0) */
    float m_k0hso12;  /** Hso(1,2) */
    float m_k0hso14;  /** Hso(1,4) */
    float m_k0hso20;  /** Hso(2,0) */
    float m_k0hso22;  /** Hso(2,2) */
    float m_k0hso24;  /** Hso(2,4) */
    float m_k0hoo0;   /** Roo(0) */
    float m_k0hoo1;   /** Roo(1) */
    float m_k0hoo2;   /** Roo(2) */
    float m_k0hoo3;   /** Roo(3) */
    float m_k0hoo4;   /** Roo(4) */

    // For final state = 1
    float m_k1mm2;    /** missing mass squared */
    float m_k1et;     /** transverse energy */
    float m_k1hso00;  /** Hso(0,0) */
    float m_k1hso01;  /** Hso(0,1) */
    float m_k1hso02;  /** Hso(0,2) */
    float m_k1hso03;  /** Hso(0,3) */
    float m_k1hso04;  /** Hso(0,4) */
    float m_k1hso10;  /** Hso(1,0) */
    float m_k1hso12;  /** Hso(1,2) */
    float m_k1hso14;  /** Hso(1,4) */
    float m_k1hso20;  /** Hso(2,0) */
    float m_k1hso22;  /** Hso(2,2) */
    float m_k1hso24;  /** Hso(2,4) */
    float m_k1hoo0;   /** Roo(0) */
    float m_k1hoo1;   /** Roo(1) */
    float m_k1hoo2;   /** Roo(2) */
    float m_k1hoo3;   /** Roo(3) */
    float m_k1hoo4;   /** Roo(4) */


    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
  public:
    /** Constructor. */
    NtupleContinuumSuppressionTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLECONTINUUMSUPPRESSIONTOOL_H
