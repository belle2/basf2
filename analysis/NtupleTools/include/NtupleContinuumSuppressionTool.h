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
    float m_fThrustB;
    float m_fThrustO;

    float m_fCosTBTO;
    float m_fCosTBz;

    float
    m_k0mm2,   m_k0et,
               m_k0hso00, m_k0hso01, m_k0hso02, m_k0hso03, m_k0hso04, m_k0hso10, m_k0hso12, m_k0hso14, m_k0hso20, m_k0hso22, m_k0hso24,
               m_k0hoo0,  m_k0hoo1,  m_k0hoo2,  m_k0hoo3,  m_k0hoo4,
               m_k1mm2,   m_k1et,
               m_k1hso00, m_k1hso01, m_k1hso02, m_k1hso03, m_k1hso04, m_k1hso10, m_k1hso12, m_k1hso14, m_k1hso20, m_k1hso22, m_k1hso24,
               m_k1hoo0,  m_k1hoo1,  m_k1hoo2,  m_k1hoo3,  m_k1hoo4;

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
