/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#ifndef VXD_DATAOBJECTS_VXDTRUEHIT_H
#define VXD_DATAOBJECTS_VXDTRUEHIT_H

#include <vxd/dataobjects/VxdID.h>
#include <framework/datastore/RelationsObject.h>
#include <TVector3.h>
#include <algorithm>

namespace Belle2 {
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
    VXDTrueHit(): RelationsObject(), m_sensorID(0), m_positionEntry(),
      m_positionMidPoint(), m_positionExit(), m_momentumEntry(),
      m_momentumMidPoint(), m_momentumExit(), m_energyDep(0), m_globalTime(0)
    {}

    /** Constructor
     * @param sensorID SensorID of the sensor
     * @param posEntry position of the start of the track in local coordinates
     * @param posMidPoint position of the mid point of the track in local coordinates
     * @param posExit position of the end of the track in local coordinates
     * @param momEntry momentum of the start of the track in local coordinates
     * @param momMidPoint momentum of the mid point of the track in local coordinates
     * @param momExit momentum of the end of the track in local coordinates
     * @param momentum momentum of the particle in local coordinates
     * @param energyDep energy deposited by the track inside the sensor
     * @param globalTime timestamp of the hit
     */
    VXDTrueHit(
      VxdID sensorID, float* posEntry, float* posMidPoint, float* posExit,
      float* momEntry, float* momMidPoint, float* momExit, float energyDep, float globalTime):
      RelationsObject(), m_sensorID(sensorID), m_energyDep(energyDep), m_globalTime(globalTime)
    {
      std::copy_n(posEntry, 3, m_positionEntry);
      std::copy_n(posMidPoint, 3, m_positionMidPoint);
      std::copy_n(posExit, 3, m_positionExit);
      std::copy_n(momEntry, 3, m_momentumEntry);
      std::copy_n(momMidPoint, 3, m_momentumMidPoint);
      std::copy_n(momExit, 3, m_momentumExit);
    }

    /** Return the Sensor ID. */
    VxdID getSensorID() const { return m_sensorID; }
    /** Return raw Sensor ID. */
    unsigned int getRawSensorID() const {return m_sensorID; }
    /** Return local u coordinate of hit.*/
    float getU() const { return m_positionMidPoint[0]; }
    /** Return local v coordinate of hit.*/
    float getV() const { return m_positionMidPoint[1]; }
    /** Return local w coordinate of hit.*/
    float getW() const { return m_positionMidPoint[2]; }
    /** Return local u coordinate of hit when entering silicon.*/
    float getEntryU() const { return m_positionEntry[0]; }
    /** Return local v coordinate of the start point of the track.*/
    float getEntryV() const { return m_positionEntry[1]; }
    /** Return local w coordinate of the start point of the track.*/
    float getEntryW() const { return m_positionEntry[2]; }
    /** Return local u coordinate of hit at the endpoint of the track.*/
    float getExitU() const { return m_positionExit[0]; }
    /** Return local v coordinate of hit at the endpoint of the track.*/
    float getExitV() const { return m_positionExit[1]; }
    /** Return local w coordinate of hit at the endpoint of the track.*/
    float getExitW() const { return m_positionExit[2]; }
    /** Return energy deposited during traversal of sensor. */
    float getEnergyDep() const { return m_energyDep; }
    /** Return the time when the track reached its midpoint.*/
    float getGlobalTime() const { return m_globalTime; }
    /** Return momentum at the midpoint of the track.*/
    TVector3 getMomentum() const { return TVector3(m_momentumMidPoint[0], m_momentumMidPoint[1], m_momentumMidPoint[2]); }
    /** Return momentum at the start point of the track.*/
    TVector3 getEntryMomentum() const { return TVector3(m_momentumEntry[0], m_momentumEntry[1], m_momentumEntry[2]);}
    /** Return momentum at the endpoint of the track.*/
    TVector3 getExitMomentum() const { return TVector3(m_momentumExit[0], m_momentumExit[1], m_momentumExit[2]); }

    /** Shift the TrueHit in time (for backgorund mixing)
     * @param delta The value of the time shift.
     */
    virtual void shiftInTime(float delta) { m_globalTime += delta; }

  protected:
    /** ID of the sensor */
    int m_sensorID;
    /** local coordinates of the start of the track inside the volume */
    float m_positionEntry[3];
    /** local coordinates of the midpoint of the track inside the volume */
    float m_positionMidPoint[3];
    /** local coordinates of the end of the track inside the volume */
    float m_positionExit[3];
    /** local momentum of the start of the track inside the volume */
    float m_momentumEntry[3];
    /** local momentum of the midpoint of the track inside the volume */
    float m_momentumMidPoint[3];
    /** local momentum of the end of the track inside the volume */
    float m_momentumExit[3];
    /** energy deposited by the track in the sensor volume */
    float m_energyDep;
    /** time when the midpoint of the track was reached */
    float m_globalTime;

    ClassDef(VXDTrueHit, 6)
  };

} // end namespace Belle2

#endif
