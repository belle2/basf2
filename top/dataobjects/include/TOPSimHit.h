/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <simulation/dataobjects/SimHitBase.h>
#include <TVector2.h>

namespace Belle2 {

  /**
   * Class to store simulated hits of Cherenkov photons on PMT's
   * input for digitization module (TOPDigitizer).
   * relation to MCParticle
   * filled in top/simulation/src/SensitivePMT.cc
   */

  class TOPSimHit : public SimHitBase {
  public:

    /**
     * Default constructor
     */
    TOPSimHit(): SimHitBase()
    {}

    /**
     * Full constructor.
     * @param moduleID    module ID
     * @param pmtID       PMT ID
     * @param xLocal      detection position x in local PMT frame
     * @param yLocal      detection position y in local PMT frame
     * @param globalTime  detection time
     * @param energy_eV   photon energy in [eV]
     */
    TOPSimHit(
      int moduleID,
      int pmtID,
      double xLocal,
      double yLocal,
      double globalTime,
      double energy_eV
    ): SimHitBase()
    {
      m_moduleID = moduleID;
      m_pmtID = pmtID;
      m_x = (float) xLocal;
      m_y = (float) yLocal;
      m_globalTime = (float) globalTime;
      m_energy = (float) energy_eV;
    }

    /**
     * Returns module ID
     * @return module ID
     */
    int getModuleID() const { return m_moduleID; }

    /**
     * Returns PMT ID
     * @return PMT ID
     */
    int getPmtID() const { return m_pmtID; }

    /**
     * Returns local position of hit (in PMT frame)
     * @return local position
     */
    TVector2 getPosition() const { return TVector2(m_x, m_y); }

    /**
     * Returns local x position of hit (in PMT frame)
     * @return local x position
     */
    double getX() const { return m_x; }

    /**
     * Returns local y position of hit (in PMT frame)
     * @return local y position
     */
    double getY() const { return m_y; }

    /**
     * Returns detection time
     * @return detection time
     */
    double getTime() const { return m_globalTime; }

    /**
     * Returns detection time (implementation of base class function)
     * @return detection time
     */
    float getGlobalTime() const override { return m_globalTime; }

    /**
     * Returns photon energy
     * @return photon energy in [eV]
     */
    double getEnergy() const { return m_energy; }

    /**
     * Shift SimHit in time (needed for beam background mixing)
     * @param delta The value of the time shift.
     */
    void shiftInTime(float delta) override { m_globalTime += delta; }

  private:

    int m_moduleID = 0;       /**< TOP module ID */
    int m_pmtID = 0;          /**< PMT ID */
    float m_x = 0;            /**< detection position in local PMT frame, x component */
    float m_y = 0;            /**< detection position in local PMT frame, y component */
    float m_globalTime = 0;   /**< detection time */
    float m_energy = 0;       /**< photon energy in [eV] */

    ClassDefOverride(TOPSimHit, 3); /**< ClassDef */

  };


} // end namespace Belle2

