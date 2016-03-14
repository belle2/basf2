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
    ECLHit(): SimHitBase() {
      m_CellId = 0;    /**< Cell ID */
      m_Edep = 0;    /**< deposited energy */
      m_TimeAve = 0; /**< average time */
      ;
    }

    //! Useful Constructor
    ECLHit(
      int CellId,
      float Edep,
      float TimeAve
    ): SimHitBase() {
      m_CellId = CellId;
      m_Edep = Edep;
      m_TimeAve = TimeAve;
    }


    /*! Set  Cell ID
     */
    void setCellId(int CellId) { m_CellId = CellId; }

    /*! Set deposit energy
     */
    void setEnergyDep(double Edep) { m_Edep = (float) Edep; }

    /*! Set  average time
     */
    void setTimeAve(double TimeAve) { m_TimeAve = (float)TimeAve; }


    /*! Get Cell ID
     * @return cell ID
     */
    int getCellId() const { return m_CellId; }

    /*! Get deposit energy
     * @return deposit energy
     */
    double getEnergyDep() const { return (double) m_Edep; }

    /*! Get average time
     * @return deposit energy
     */
    double getTimeAve() const {return (double) m_TimeAve;}

    /*! Get average time (implementation of base class function)
     *  @return average time
     *  by Marko Staric */
    float getGlobalTime() const {return m_TimeAve;}

    /** Shift the Hit in time (needed for beam background mixing)
     * @param delta The value of the time shift.
     */
    void shiftInTime(float delta) {  m_TimeAve += delta; }


  private:

    int m_CellId;      /**< Cell ID */
    float m_Edep;      /**< deposited energy */
    float m_TimeAve;   /**< average time */

    ClassDef(ECLHit, 3); /**< ClassDef */

  };

} // end namespace Belle2

#endif
