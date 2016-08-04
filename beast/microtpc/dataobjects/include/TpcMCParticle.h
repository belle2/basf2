/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TPCMCPARTICLE_H
#define TPCMCPARTICLE_H

#include <simulation/dataobjects/SimHitBase.h>

// ROOT
#include <TObject.h>
#include <TVector3.h>
#include <vector>

namespace Belle2 {

  /**
    * Class TpcMCParticle - Geant4 simulated hit for the Microtpc detector.
    *
    * This class holds particle hit data from geant4 simulation. As the simulated
    * hit classes are used to generate detector response, they contain _local_
    * information.
    */
  class TpcMCParticle : public SimHitBase {
  public:

    /** default constructor for ROOT */
    TpcMCParticle(): m_PDG(0), m_Mass(0), m_Energy(0), m_vtx(), m_mom() {}


    /** Standard constructor
     * @param energyDep Deposited energy in electrons
     */
    TpcMCParticle(int PDG, float Mass, float Energy, const float* vtx, const float* mom):
      SimHitBase(), m_PDG(PDG), m_Mass(Mass), m_Energy(Energy)
    {
      std::copy(vtx, vtx + 3, m_vtx);
      std::copy(mom, mom + 3, m_mom);
    }

    /** Return PDG */
    int getPDG()      const { return m_PDG; }
    /** Return mass */
    float getMass()      const { return m_Mass; }
    /** Return energy */
    float getEnergy()      const { return m_Energy; }
    /** Return production vertex */
    TVector3 getProductionVertex()  const { return TVector3(m_vtx[0], m_vtx[1], m_vtx[2]); }
    /** Return momentum */
    TVector3 getMomentum() const { return TVector3(m_mom[0], m_mom[1], m_mom[2]); }
    /** Return production array as float array */
    const float* getProductionVertexArray() const { return m_vtx; }
    /** Return momentum as float array */
    const float* getMomentumArray() const { return m_mom; }

  private:
    /** PDG */
    int m_PDG;
    /** Mass */
    float m_Mass;
    /** Energy */
    float m_Energy;
    /** Vertex */
    float m_vtx[3];
    /** Momentum */
    float m_mom[3];

    ClassDef(TpcMCParticle, 1)
  };

} // end namespace Belle2

#endif
