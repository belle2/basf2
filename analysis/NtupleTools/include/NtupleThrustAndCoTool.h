/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Ami Rostomyan                                            *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLETHRUSTANDCOTOOL_H
#define NTUPLETHRUSTANDCOTOOL_H

#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>

#include <TTree.h>
#include <string>
#include <utility>


namespace Belle2 {

  /**
   * NtupleTool to write out the thrust axis, thrust value and the cosine angle between the particle and the thrust axis
   */
  class NtupleThrustAndCoTool : public NtupleFlatTool {

  private:
    float* m_fThrustValue;         /**< The thrust value of the event */
    float m_fThrustVector[3];      /**< The thrust vector. */
    float* m_fCosThrust;           /**< Cosine of the angle between the thrust axis and the particle's momentum */

    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
    /** delete 'new's */
    void deallocateMemory();
  public:
    /** Constuctor. */
    NtupleThrustAndCoTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}
    /** Destructor. */
    ~NtupleThrustAndCoTool() {deallocateMemory();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLETHRUSTANDCO_H
