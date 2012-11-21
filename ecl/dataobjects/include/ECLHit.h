/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLHIT_H
#define ECLHIT_H

#include <framework/datastore/DataStore.h>
#include <TVector3.h>
#include <generators/dataobjects/SimHitBase.h>

namespace Belle2 {
  /** \addtogroup dataobjects
   * @{
   */

  /*! Class to store simulated hits which equate to average of ECLSImHit  on crystals
   * input for digitization module (ECLDigitizer).
   * relation to MCParticle
   * filled in ecl/simulation/src/SensitiveDetector.cc
   */

  class ECLHit : public SimHitBase {
  public:
    /** default constructor for ROOT */
    ECLHit() {;}

    /*! Set  Cell ID
     */
    void setCellId(int cellId) { m_cellId = cellId; }

    /*! Set deposit energy
     */
    void setEnergyDep(double edep) { m_edep = edep; }

    /*! Set  average time
     */
    void setTimeAve(double TimeAve) { m_TimeAve = TimeAve; }


    /*! Get Cell ID
     * @return cell ID
     */
    int getCellId() const { return m_cellId; }

    /*! Get deposit energy
     * @return deposit energy
     */
    double getEnergyDep() const { return m_edep; }

    /*! Get average time
     * @return deposit energy
     */
    double getTimeAve() const {return m_TimeAve;}

    /** Shift the Hit in time (needed for beam background mixing)
     * @param delta The value of the time shift.
     */
    void shiftInTime(float delta) {  m_TimeAve += delta; }


  private:

    int m_cellId;      /**< Cell ID */
    double m_edep;     /**< deposited energy */
    double m_TimeAve;  /**< average time */

    ClassDef(ECLHit, 3); /**< ClassDef */

  };

  /** @}*/
} // end namespace Belle2

#endif
