/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *               Alexandre Beaulieu                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CSIDIGIT_H
#define CSIDIGIT_H

#include <framework/datastore/RelationsObject.h>
namespace Belle2 {

  /*! Class to store Csi digitized hits (output of CsIDigitizer)
   * relation to CsiHit
   * filled in beast/csi/modules/src/CsIDigitizerModule.cc
   */

  class CsiDigiHit : public RelationsObject {
  public:
    /** default constructor for ROOT */
    CsiDigiHit() {
      m_CellId = 0;    /**< Cell ID */
      m_Amp = 0;       /**< Fitting Amplitude */
      m_TimeFit = 0;   /**< Fitting Time */
      m_Quality = 0;   /**< Fitting Quality */

      ;
    }

    /*! Set  Cell ID
     */
    void setCellId(int CellId) { m_CellId = CellId; }

    /*! Set Fitting Amplitude
     */
    void setAmp(int Amp) { m_Amp = Amp; }


    /*! Set Fitting Time
     */
    void setTimeFit(int TimeFit) { m_TimeFit = TimeFit; }


    /*! Set Fitting Quality
     */
    void setQuality(int Quality) { m_Quality = Quality; }


    /*! Get Cell ID
     * @return cell ID
     */
    int getCellId() const { return m_CellId; }


    /*! Get Fitting Amplitude
     * @return Fitting Amplitude
     */
    int getAmp() const { return m_Amp; }

    /*! Get Fitting Time
     * @return Fitting Time
     */
    int getTimeFit() const { return m_TimeFit; }

    /*! Get Fitting Quality
     * @return Fitting Quality
     */
    int getQuality() const { return m_Quality; }

  private:

    int m_CellId;      /**< Cell ID */
    int m_Amp;         /**< Fitting Amplitude */
    int m_TimeFit;     /**< Fitting Time */
    int m_Quality;     /**< Fitting Quality */


    ClassDef(CsiDigiHit, 1);/**< ClassDef */

  };
} // end namespace Belle2

#endif
