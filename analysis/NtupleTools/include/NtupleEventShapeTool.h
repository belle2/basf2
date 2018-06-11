/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2018 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Ami Rostomyan                                            *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEEVENTSHAPETOOL_H
#define NTUPLEEVENTSHAPETOOL_H

#include <analysis/NtupleTools/NtupleFlatTool.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>

#include <TTree.h>
#include <string>
#include <utility>


namespace Belle2 {

  /**
   * NtupleTool to write out the thrust axis, the cosine angle between the particle and the thrust axis and the missing
   * momentum.
   */
  class NtupleEventShapeTool : public NtupleFlatTool {

  private:
    float m_fThrustValue;            /**< The thrust value of the event */
    float m_fThrustVector[3];        /**< The thrust vector. */
    float* m_fCosToThrust;           /**< Cosine of the angle between the thrust axis and the particle's momentum */
    float m_fMissingMomentum[3];     /**< The missing momentum of the event in the lab system */
    float m_fMissingMomentumCMS[3];  /**< The missing momentum of the event in the CMS  */
    float m_fMissingEnergyCMS;       /**< The missing energy of the event in the CMS  */
    float m_fMissingMass2 ;          /**< The missing mass squared  */
    float m_fVisibleEnergyCMS;       /**< The visible energy of the event in the CMS  */
    float m_fTotalPhotonsEnergy;     /**< Total energy of the photons in the event in lab system  */

    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
    /** delete 'new's */
    void deallocateMemory();
  public:
    /** Constuctor. */
    NtupleEventShapeTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}
    /** Destructor. */
    ~NtupleEventShapeTool() {deallocateMemory();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };

} // namepspace Belle2

#endif // NTUPLETHRUSTANDCO_H
