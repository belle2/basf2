/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef CSIHIT_H
#define CSIHIT_H

#include <simulation/dataobjects/SimHitBase.h>

namespace Belle2 {

  /*! Class to store simulated hits which equate to average of CsiSimHit on crystals
   * Input for digitization module (CsiDigitizer).
   * Relation to MCParticle.
   * Filled in beast/csi/simulation/src/SensitiveDetector.cc
   */

  class CsiHit : public SimHitBase {
  public:
    /** default constructor for ROOT */
    CsiHit(): SimHitBase()
    {
      m_CellId = 0;    /**< Cell ID */
      m_Edep = 0;    /**< deposited energy */
      m_TimeAve = 0; /**< average time */
      m_TimeVar = 0; /**< time variance */
      ;
    }

    //! Useful Constructor
    CsiHit(
      int CellId,
      float Edep,
      float TimeAve,
      float TimeVar
    ): SimHitBase()
    {
      m_CellId = CellId;
      m_Edep = Edep;
      m_TimeAve = TimeAve;
      m_TimeVar = TimeVar;
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

    /*! Set time variance
     */
    void setTimeVar(double TimeVar) { m_TimeVar = (float)TimeVar; }


    /*! Get Cell ID
     * @return cell ID
     */
    int getCellId() const { return m_CellId; }

    /*! Get deposit energy
     * @return deposit energy
     */
    double getEnergyDep() const { return (double) m_Edep; }

    /*! Get average time
     * @return average time
     */
    double getTimeAve() const {return (double) m_TimeAve;}

    /*! Get time variance
     * @return (time variance) * (total deposited energy)
     */
    double getTimeVar() const {return (double) m_TimeVar;}


    /** Shift the Hit in time (needed for beam background mixing)
     * @param delta The value of the time shift.
     */
    void shiftInTime(float delta) override {  m_TimeAve += delta; }


  private:

    int m_CellId;      /**< Cell ID */
    float m_Edep;      /**< deposited energy */
    float m_TimeAve;   /**< average time */
    float m_TimeVar;   /**< time variance*/

    ClassDefOverride(CsiHit, 3); /**< ClassDef */

  };

} // end namespace Belle2

#endif
