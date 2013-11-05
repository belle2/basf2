/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Luigi Li Gioi                                            *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEVERTEXTOOL_H
#define NTUPLEVERTEXTOOL_H
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

using namespace std;
using namespace Belle2;

namespace Belle2 {

  /** Writes vertex information of B candidate to flat ntuple. */
  class NtupleVertexTool : public NtupleFlatTool {
  private:
    /** Vertex position X*/
    float* m_fDX;
    /** Vertex position Y*/
    float* m_fDY;
    /** Vertex position Z*/
    float* m_fDZ;
    /** Vertex position Sqrt(X^2+Y^2)*/
    float* m_fDRho;
    /** Vertex fit P-value */
    float* m_fPvalue;
    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
  public:
    /** Constructor. */
    NtupleVertexTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor), m_fDX(0), m_fDY(0), m_fDZ(0), m_fDRho(0), m_fPvalue(0) {setupTree();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLEVERTEXTOOL_H
