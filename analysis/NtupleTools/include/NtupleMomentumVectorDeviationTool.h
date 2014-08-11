/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Anze Zupanc                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEMOMENTUMVECTORDEVIATIONTOOL_H
#define NTUPLEMOMENTUMVECTORDEVIATIONTOOL_H
#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TTree.h>
#include <string>
#include <utility>

namespace Belle2 {

  /**
   * Writes the angle between the 3-momentum vector and the vector
   * joining particle's production and decay vertices.
   */

  class NtupleMomentumVectorDeviationTool : public NtupleFlatTool {
  private:

    /** angle between the 3-momentum vector and the vector joining particle's production and decay vertices */
    float m_cosAngle;

    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();

  public:

    /** Constructor. */
    NtupleMomentumVectorDeviationTool(TTree* tree, DecayDescriptor& decaydescriptor) :  NtupleFlatTool(tree, decaydescriptor)
      , m_cosAngle(0) {
      setupTree();
    }

    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);

  };

} // namepspace Belle2

#endif // NTUPLEMOMENTUMVECTORDEVIATIONTOOL_H
