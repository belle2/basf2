/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Peter Kvasnicka, Martin Ritter             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VXDTRUEHIT_H
#define VXDTRUEHIT_H

#include <vxd/dataobjects/VxdID.h>
#include <framework/datastore/RelationsObject.h>
#include <TVector3.h>

namespace Belle2 {
  /** \addtogroup dataobjects
    * @{
    */

  /**
    * Class VXDTrueHit - Records of tracks that either enter or leave
    * the sensitive volume.
    *
    * This class is meant as helper for tracking optimization. It stores
    * information about particle tracks in a sensor in condensed form: The local
    * coordinates of the start point, midpoint and endpoint of the track,
    * as well as track momenta at these points.
    *
    * This class holds particle hit data from geant4 simulation. As the simulated
    * hit classes are used to generate detector response, they contain _local_
    * information.
    */
  class VXDTrueHit : public RelationsObject {

  public:
    /** Default constructor for ROOT IO */
    VXDTrueHit(): RelationsObject(), m_sensorID(0), m_u(0), m_v(0), m_w(0),
      m_entryU(0), m_entryV(0), m_entryW(0), m_exitU(0), m_exitV(0), m_exitW(0),
      m_energyDep(0), m_globalTime(0) {}

    /** Constructor
     * @param sensorID SensorID of the Sensor
     * @param u u coordinate of the hit in local coordinates
     * @param v v coordinate of the hit in local coordinates
     * @param momentum momentum of the particle in local coordinates
     * @param entryMomentum momentum of the particle in local coordinates when entering silicon
     * @param exitMomentum momentum of the particle in local coordinates when exiting silicon
     * @param globalTime timestamp of the hit
     * NOTE: This constructor sets entry and exit coordinates to u,v and all w's
     * to 0.
     */
    VXDTrueHit(
      VxdID sensorID, float u, float v, float energyDep, float globalTime,
      const TVector3& momentum, const TVector3& entryMomentum, const TVector3& exitMomentum):
      RelationsObject(),
      m_sensorID(sensorID), m_u(u), m_v(v), m_w(0), m_entryU(u), m_entryV(v), m_entryW(0),
      m_exitU(u), m_exitV(v), m_exitW(0), m_energyDep(energyDep), m_globalTime(globalTime),
      m_momentum(momentum), m_entryMomentum(entryMomentum), m_exitMomentum(exitMomentum) {}

    /** (Full) constructor
     * @param sensorID SensorID of the sensor
     * @param u u coordinate of the hit
     * @param v v coordinate of the hit
     * @param w w coordinate of the hit
     * @param entryU u coordinate of the track start point
     * @param entryV v coordinate of the track start point
     * @param entryW w coordinate of the track start point
     * @param exitU u coordinate of the track endpoint
     * @param exitV v coordinate of the track endpoint
     * @param exitW w coordinate of the track endpoint
     * @param momentum momentum of the particle in local coordinates
     * @param entryMomentum momentum at track start point
     * @param exitMomentum momentum at track endpoint
     * @param globalTime timestamp of the hit
     */
    VXDTrueHit(
      VxdID sensorID, float u, float v, float w, float entryU, float entryV, float entryW,
      float exitU, float exitV, float exitW, float energyDep, float globalTime,
      const TVector3& momentum, const TVector3& entryMomentum, const TVector3& exitMomentum):
      RelationsObject(),
      m_sensorID(sensorID), m_u(u), m_v(v), m_w(w), m_entryU(entryU), m_entryV(entryV), m_entryW(entryW),
      m_exitU(exitU), m_exitV(exitV), m_exitW(exitW),  m_energyDep(energyDep), m_globalTime(globalTime),
      m_momentum(momentum), m_entryMomentum(entryMomentum), m_exitMomentum(exitMomentum) {}

    /** Return the Sensor ID. */
    VxdID getSensorID() const { return m_sensorID; }
    /** Return raw Sensor ID. */
    unsigned int getRawSensorID() const {return m_sensorID; }
    /** Return local u coordinate of hit.*/
    float getU() const { return m_u; }
    /** Return local v coordinate of hit.*/
    float getV() const { return m_v; }
    /** Return local w coordinate of hit.*/
    float getW() const { return m_w; }
    /** Return local u coordinate of hit when entering silicon.*/
    float getEntryU() const { return m_entryU; }
    /** Return local v coordinate of the start point of the track.*/
    float getEntryV() const { return m_entryV; }
    /** Return local w coordinate of the start point of the track.*/
    float getEntryW() const { return m_entryW; }
    /** Return local u coordinate of hit at the endpoint of the track.*/
    float getExitU() const { return m_exitU; }
    /** Return local v coordinate of hit at the endpoint of the track.*/
    float getExitV() const { return m_exitV; }
    /** Return local w coordinate of hit at the endpoint of the track.*/
    float getExitW() const { return m_exitW; }
    /** Return energy deposited during traversal of sensor. */
    float getEnergyDep() const { return m_energyDep; }
    /** Return the time when the track reached its midpoint.*/
    float getGlobalTime() const { return m_globalTime; }
    /** Return momentum at the midpoint of the track.*/
    const TVector3& getMomentum() const { return m_momentum; }
    /** Return momentum at the start point of the track.*/
    const TVector3& getEntryMomentum() const { return m_entryMomentum; }
    /** Return momentum at the endpoint of the track.*/
    const TVector3& getExitMomentum() const { return m_exitMomentum; }

  protected:

    int m_sensorID;           /**< ID of the sensor */
    float m_u;                /**< Mean local u coordinate of the track */
    float m_v;                /**< Mean local v coordinate of the track */
    float m_w;          /**< Mean local w coordinate of the track */
    float m_entryU;           /**< Local u coordinate of the track start point */
    float m_entryV;           /**< Local v coordinate of the track start point */
    float m_entryW;           /**< Local w coordinate of the track start point */
    float m_exitU;            /**< Local u coordinate of the track end point*/
    float m_exitV;            /**< Local v coordinate of the track end point*/
    float m_exitW;            /**< Local w coordinate of the track end point */
    float m_energyDep;        /**< Energy deposited by the track in the sensor */
    float m_globalTime;       /**< Time when midpoint of the track was reached. */
    TVector3 m_momentum;      /**< momentum in local coordinates at track midpoint */
    TVector3 m_entryMomentum; /**< momentum in local coordinates at track start */
    TVector3 m_exitMomentum;  /**< momentum in local coordinates at track end */

    ClassDef(VXDTrueHit, 4)
  };

  /** @}*/

} // end namespace Belle2

#endif
