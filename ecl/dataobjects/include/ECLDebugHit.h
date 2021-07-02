/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include  <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /*! Class to store the average of ECLSimHit  on crystals for debug only
   * input for digitization module (ECLDigitizer).
   * relation to MCParticle
   * filled in ecl/simulation/src/SensitiveDetector.cc
   */

  class ECLDebugHit : public RelationsObject {
  public:
    /** default constructor for ROOT */
    ECLDebugHit()
    {
      m_cellId = 0;    /**< Cell ID */
      m_Edep = 0;    /**< deposited energy */
      m_TimeAve = 0; /**< average time */
    }

    /*! Set  Cell ID
     */
    void setCellId(int cellId) { m_cellId = cellId; }

    /*! Set deposit energy
     */
    void setEnergyDep(double Edep) { m_Edep = (float) Edep; }

    /*! Set  average time
     */
    void setTimeAve(double TimeAve) { m_TimeAve = (float)TimeAve; }


    /*! Get Cell ID
     * @return cell ID
     */
    int getCellId() const { return m_cellId; }

    /*! Get deposit energy
     * @return deposit energy
     */
    double getEnergyDep() const { return (double) m_Edep; }

    /*! Get average time
     * @return deposit energy
     */
    double getTimeAve() const {return (double) m_TimeAve;}

    /** Shift the Hit in time (needed for beam background mixing)
     * @param delta The value of the time shift.
     */
    void shiftInTime(float delta) {  m_TimeAve += delta; }


  private:

    int m_cellId;      /**< Cell ID */
    float m_Edep;      /**< deposited energy */
    float m_TimeAve;   /**< average time */

    ClassDef(ECLDebugHit, 1); /**< ClassDef */

  };

} // end namespace Belle2

