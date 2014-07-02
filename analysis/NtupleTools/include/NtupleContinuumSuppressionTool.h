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
    float m_k0et;     /** E_{T} */
    float m_k0hso00;  /** H_{00}^{so} */
    float m_k0hso01;  /** H_{01}^{so} */
    float m_k0hso02;  /** H_{02}^{so} */
    float m_k0hso03;  /** H_{03}^{so} */
    float m_k0hso04;  /** H_{04}^{so} */
    float m_k0hso10;  /** H_{10}^{so} */
    float m_k0hso12;  /** H_{12}^{so} */
    float m_k0hso14;  /** H_{14}^{so} */
    float m_k0hso20;  /** H_{20}^{so} */
    float m_k0hso22;  /** H_{22}^{so} */
    float m_k0hso24;  /** H_{24}^{so} */
    float m_k0hoo0;   /** R_{0}^{oo} */
    float m_k0hoo1;   /** R_{1}^{oo} */
    float m_k0hoo2;   /** R_{2}^{oo} */
    float m_k0hoo3;   /** R_{3}^{oo} */
    float m_k0hoo4;   /** R_{4}^{oo} */

    // For final state = 1
    float m_k1mm2;    /** missing mass squared */
    float m_k1et;     /** E_{T} */
    float m_k1hso00;  /** H_{00}^{so} */
    float m_k1hso01;  /** H_{01}^{so} */
    float m_k1hso02;  /** H_{02}^{so} */
    float m_k1hso03;  /** H_{03}^{so} */
    float m_k1hso04;  /** H_{04}^{so} */
    float m_k1hso10;  /** H_{10}^{so} */
    float m_k1hso12;  /** H_{12}^{so} */
    float m_k1hso14;  /** H_{14}^{so} */
    float m_k1hso20;  /** H_{20}^{so} */
    float m_k1hso22;  /** H_{22}^{so} */
    float m_k1hso24;  /** H_{24}^{so} */
    float m_k1hoo0;   /** R_{0}^{oo} */
    float m_k1hoo1;   /** R_{1}^{oo} */
    float m_k1hoo2;   /** R_{2}^{oo} */
    float m_k1hoo3;   /** R_{3}^{oo} */
    float m_k1hoo4;   /** R_{4}^{oo} */


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
