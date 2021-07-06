/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <simulation/dataobjects/SimHitBase.h>

namespace Belle2 {

  /*! Class to store simulated hits which equate to average of ECLSImHit  on crystals
   * input for digitization module (ECLDigitizer).
   * relation to MCParticle
   * filled in ecl/simulation/src/SensitiveDetector.cc
   */

  class ECLHit : public SimHitBase {
  public:
    /** default constructor for ROOT */
    ECLHit(): SimHitBase()
    {
      m_CellId = 0;    /**< Cell ID */
      m_Edep = 0;    /**< deposited energy */
      m_TimeAve = 0; /**< average time */
      ;
    }

    //! Useful Constructor
    ECLHit(
      int CellId,
      double Edep,
      double TimeAve
    ): SimHitBase()
    {
      m_CellId = CellId;
      m_Edep = Edep;
      m_TimeAve = TimeAve;
    }


    /*! Set  Cell ID
     */
    void setCellId(int CellId) { m_CellId = CellId; }

    /*! Set deposit energy
     */
    void setEnergyDep(double Edep) { m_Edep = static_cast<double>(Edep); }

    /*! Set  average time
     */
    void setTimeAve(double TimeAve) { m_TimeAve = static_cast<double>(TimeAve); }


    /*! Get Cell ID
     * @return cell ID
     */
    int getCellId() const { return m_CellId; }

    /*! Get deposit energy
     * @return deposit energy
     */
    double getEnergyDep() const { return m_Edep; }

    /*! Get average time
     * @return deposit energy
     */
    double getTimeAve() const { return m_TimeAve; }

    /*! Get average time (implementation of base class function)
     *  @return average time
     */
    float getGlobalTime() const override { return m_TimeAve; }

    /** Shift the Hit in time (needed for beam background mixing)
     * @param delta The value of the time shift.
     */
    void shiftInTime(float delta) override {  m_TimeAve += delta; }


  private:

    int m_CellId;      /**< Cell ID */
    double m_Edep;      /**< deposited energy */
    double m_TimeAve;   /**< average time */

    ClassDefOverride(ECLHit, 4); /**< ClassDef */

  };

} // end namespace Belle2

