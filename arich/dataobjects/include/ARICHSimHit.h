/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef ARICHSIMHIT_H
#define ARICHSIMHIT_H

#include <simulation/dataobjects/SimHitBase.h>

#include <TVector2.h>

namespace Belle2 {


  //! Class ARICHSimHit - Geant4 simulated hit for ARICH.
  /*!
    This is a class to store ARICH geant4 hit in datastore.
    It is also the input for digitization module (ARICHDigi).
   */

  class ARICHSimHit : public SimHitBase {
  public:

    //! Default constructor
    ARICHSimHit(): SimHitBase(),
      m_moduleID(0),
      m_x(0.0),
      m_y(0.0),
      m_globalTime(0.0),
      m_energy(0.0)
    {
      /*! Does nothing */
    }

    //! Full constructor.
    /*!
      \param moduleID ID of hapd module containing hit
      \param position vector of hit local position (in module coor. sys.)
      \param globalTime global time of photon hit
      \param energy energy of photon
      \param parentID geant4 id of photon parent particle
    */

    ARICHSimHit(
      int moduleID,
      TVector2 position,
      double globalTime,
      double energy): SimHitBase()
    {
      m_moduleID = moduleID;
      m_x = (float) position.X();
      m_y = (float) position.Y();
      m_globalTime = (float) globalTime;
      m_energy = (float) energy;
    }

    //! Get ID number of module that registered hit
    int getModuleID() const { return m_moduleID; }

    //! Get local position of hit (in module coordinates)
    TVector2 getLocalPosition() const { TVector2 vec(m_x, m_y); return vec; }

    //! Get global time of hit
    float getGlobalTime() const override { return m_globalTime; }

    //! Get detected photon energy
    double getEnergy() const { return m_energy; }

    /** Shift the SimHit in time (needed for beam background mixing)
     *  @param delta The value of the time shift.
     */
    void shiftInTime(float delta) override { m_globalTime += delta;}


  private:
    int m_moduleID;          /**< ID number of module that registered hit*/
    float m_x;               /**< Local position of hit, x component (in module coordinates) */
    float m_y;               /**< Local position of hit, y component (in module coordinates) */
    float m_globalTime;      /**< Global time of hit */
    float m_energy;          /**< Energy of detected photon */

    ClassDefOverride(ARICHSimHit, 1); /**< the class title */

  };

} // end namespace Belle2

#endif
