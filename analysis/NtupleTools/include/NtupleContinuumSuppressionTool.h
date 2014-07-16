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

  /** Writes continuum suppression variables to flat ntuple. */
  class NtupleContinuumSuppressionTool : public NtupleFlatTool {
  private:
    float m_fThrustB; /** magnitude of B thrust axis */
    float m_fThrustO; /** magnitude of ROE thrust axis */

    float m_fCosTBTO; /** cosine of the angle between the thrust axis of the B and the thrust axis of the ROE */
    float m_fCosTBz;  /** cosine of the angle between the thrust axis of the B and the z-axis */

    float m_fR2;      /** reduced Fox-Wolfram moment R2 */

    // For final state = 0
    float m_fk0mm2;    /** missing mass squared */
    float m_fk0et;     /** transverse energy */
    float m_fk0hso00;  /** Hso(0,0) */
    float m_fk0hso01;  /** Hso(0,1) */
    float m_fk0hso02;  /** Hso(0,2) */
    float m_fk0hso03;  /** Hso(0,3) */
    float m_fk0hso04;  /** Hso(0,4) */
    float m_fk0hso10;  /** Hso(1,0) */
    float m_fk0hso12;  /** Hso(1,2) */
    float m_fk0hso14;  /** Hso(1,4) */
    float m_fk0hso20;  /** Hso(2,0) */
    float m_fk0hso22;  /** Hso(2,2) */
    float m_fk0hso24;  /** Hso(2,4) */
    float m_fk0hoo0;   /** Roo(0) */
    float m_fk0hoo1;   /** Roo(1) */
    float m_fk0hoo2;   /** Roo(2) */
    float m_fk0hoo3;   /** Roo(3) */
    float m_fk0hoo4;   /** Roo(4) */

    // For final state = 1
    float m_fk1mm2;    /** missing mass squared */
    float m_fk1et;     /** transverse energy */
    float m_fk1hso00;  /** Hso(0,0) */
    float m_fk1hso01;  /** Hso(0,1) */
    float m_fk1hso02;  /** Hso(0,2) */
    float m_fk1hso03;  /** Hso(0,3) */
    float m_fk1hso04;  /** Hso(0,4) */
    float m_fk1hso10;  /** Hso(1,0) */
    float m_fk1hso12;  /** Hso(1,2) */
    float m_fk1hso14;  /** Hso(1,4) */
    float m_fk1hso20;  /** Hso(2,0) */
    float m_fk1hso22;  /** Hso(2,2) */
    float m_fk1hso24;  /** Hso(2,4) */
    float m_fk1hoo0;   /** Roo(0) */
    float m_fk1hoo1;   /** Roo(1) */
    float m_fk1hoo2;   /** Roo(2) */
    float m_fk1hoo3;   /** Roo(3) */
    float m_fk1hoo4;   /** Roo(4) */

    float m_fcc1;      /** Cleo Cone 1 */
    float m_fcc2;      /** Cleo Cone 2 */
    float m_fcc3;      /** Cleo Cone 3 */
    float m_fcc4;      /** Cleo Cone 4 */
    float m_fcc5;      /** Cleo Cone 5 */
    float m_fcc6;      /** Cleo Cone 6 */
    float m_fcc7;      /** Cleo Cone 7 */
    float m_fcc8;      /** Cleo Cone 8 */
    float m_fcc9;      /** Cleo Cone 9 */

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
