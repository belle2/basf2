/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPSIMHIT_H
#define TOPSIMHIT_H

#include <simulation/dataobjects/SimHitBase.h>
#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  /*! Class to store simulated hits of Cherenkov photons on PMT's
   * input for digitization module (TOPDigitizer).
   * relation to MCParticle
   * filled in top/simulation/src/SensitivePMT.cc
   */

  class TOPSimHit : public SimHitBase {
  public:

    /*! Default constructor
     */
    TOPSimHit():
      SimHitBase(),
      m_barID(0),
      m_pmtID(0),
      m_x(0.0),
      m_y(0.0),
      m_globalTime(0.0),
      m_energy(0.0) {
    }

    /*!  Full constructor.
     * @param barID       bar ID
     * @param pmtID       PMT ID
     * @param xLocal      detection position x in local PMT frame
     * @param yLocal      detection position y in local PMT frame
     * @param globalTime  detection time
     * @param energy_eV   photon energy in [eV]
     */
    TOPSimHit(
      int barID,
      int pmtID,
      double xLocal,
      double yLocal,
      double globalTime,
      double energy_eV
    ): SimHitBase() {
      m_barID = barID;
      m_pmtID = pmtID;
      m_x = (float) xLocal;
      m_y = (float) yLocal;
      m_globalTime = (float) globalTime;
      m_energy = (float) energy_eV;
    }

    /*! Get bar ID
     * @return bar ID
     */
    int getBarID() const { return m_barID; }

    /*! Get PMT ID
     * @return PMT ID
     */
    int getPmtID() const { return m_pmtID; }

    /*! Get local position of hit (in PMT frame)
     * @return local position
     */
    TVector2 getPosition() const { TVector2 vec(m_x, m_y); return vec; }

    /*! Get local x position of hit (in PMT frame)
     * @return local x position
     */
    double getX() const { return m_x; }

    /*! Get local y position of hit (in PMT frame)
     * @return local y position
     */
    double getY() const { return m_y; }

    /*! Get detection time
     * @return detection time
     */
    double getTime() const { return m_globalTime; }

    /*! Get photon energy
     * @return photon energy in [eV]
     */
    double getEnergy() const { return m_energy; }

    /** Shift the SimHit in time (needed for beam background mixing)
     * @param delta The value of the time shift.
     */
    void shiftInTime(float delta) { m_globalTime += delta; }

  private:

    int m_barID;          /**< bar ID */
    int m_pmtID;          /**< PMT ID */
    float m_x;            /**< detection position in local PMT frame, x component */
    float m_y;            /**< detection position in local PMT frame, y component */
    float m_globalTime;   /**< detection time */
    float m_energy;       /**< photon energy in [eV] */

    ClassDef(TOPSimHit, 2); /**< ClassDef */

  };


} // end namespace Belle2

#endif
