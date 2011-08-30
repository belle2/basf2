/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Peter Kvasnicka, Martin Ritter             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDTRUEHIT_H
#define SVDTRUEHIT_H

#include <pxd/vxd/VxdID.h>

// ROOT
#include <TObject.h>
#include <TVector3.h>

#define DEFAULT_SVDTRUEHITS "SVDTrueHits"
#define DEFAULT_SVDTRUEHITSREL "MCParticlesToSVDTrueHits"

namespace Belle2 {

  /**
    * ClassSVDTrueHit - Geant4 simulated hit for the SVD.
    *
    * This class holds particle hit data from geant4 simulation. As the simulated
    * hit classes are used to generate detector response, they contain _local_
    * information.
    */
  class SVDTrueHit : public TObject {

  public:
    SVDTrueHit(
      VxdID vxdID = 0,
      float u = 0, float v = 0, const TVector3 momentum = TVector3(0, 0, 0), float globalTime = 0):
        m_vxdID(vxdID),
        m_u(u), m_v(v), m_momentum(momentum), m_globalTime(globalTime) {}

    /** Return the Sensor ID */
    VxdID getSensorID() const { return m_vxdID; }
    /** Retun local u coordinate of hit */
    float getU() const { return m_u; }
    /** Retun local v coordinate of hit */
    float getV() const { return m_v; }
    /** The method to get momentum.*/
    const TVector3& getMomentum() const { return m_momentum; }
    /** The method to get GlobalTime.*/
    float getGlobalTime() const { return m_globalTime; }

  private:

    int m_vxdID;             /**< ID of the sensor */
    float m_u;               /**< Local u coordinate */
    float m_v;               /**< Local v coordinate */
    TVector3 m_momentum;     /**< momentum in local coordinates */
    float m_globalTime;      /**< Global time. */

    ClassDef(SVDTrueHit, 1)
  };

} // end namespace Belle2

#endif
