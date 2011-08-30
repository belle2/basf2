/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Peter Kvasnicka                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDSIMHIT_H
#define SVDSIMHIT_H

// ROOT
#include <TObject.h>
#include <TVector3.h>
#include <pxd/vxd/VxdID.h>

namespace Belle2 {

  /**
    * ClassSVDSimHit - Geant4 simulated hit for the SVD.
    *
    * This class holds particle hit data from geant4 simulation. As the simulated
    * hit classes are used to generate detector response, they contain _local_
    * information.
    */
  class SVDSimHit : public TObject {
  public:
    SVDSimHit(): m_vxdID(0), m_pdg(0), m_theta(0), m_energyDep(0), m_globalTime(0),
        m_posIn(0, 0, 0), m_posOut(0, 0, 0), m_momIn(0, 0, 0) {}

    SVDSimHit(VxdID vxdID, int pdg, float theta, float energyDep, float globalTime,
              const TVector3& posIn, const TVector3& posOut, const TVector3& momIn):
        m_vxdID(vxdID), m_pdg(pdg), m_theta(theta), m_energyDep(energyDep), m_globalTime(globalTime),
        m_posIn(posIn), m_posOut(posOut), m_momIn(momIn) {}

    VxdID getSensorID()         const { return m_vxdID; };
    const TVector3& getPosIn()  const { return m_posIn; }
    const TVector3& getPosOut() const { return m_posOut; }
    const TVector3& getMomIn()  const { return m_momIn; }
    float getTheta()            const { return m_theta; }
    float getEnergyDep()        const { return m_energyDep; }
    float getGlobalTime()       const { return m_globalTime; }
  private:
    unsigned short m_vxdID;
    int m_pdg;
    float m_theta;
    float m_energyDep;
    float m_globalTime;
    TVector3 m_posIn;
    TVector3 m_posOut;
    TVector3 m_momIn;

    ClassDef(SVDSimHit, 1)
  }; // class SVDSimHit

} // namespace Belle2

#endif // SVDSIMHIT_H
