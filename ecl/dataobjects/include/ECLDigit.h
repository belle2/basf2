/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
namespace Belle2 {

  /*! Class to store ECL digitized hits (output of ECLDigi)
   * relation to ECLHit
   * filled in ecl/modules/eclDigitizer/src/ECLDigitizerModule.cc
   */

  class ECLDigit : public RelationsObject {
  public:
    /** default constructor for ROOT */
    ECLDigit()
    {
      m_CellId = 0;    /**< Cell ID */
      m_Amp = 0;       /**< Fitting Amplitude */
      m_TimeFit = 0;   /**< Fitting Time */
      m_Quality = 0;   /**< Fitting Quality */
      m_Chi = 0;      /**< Fitting chisquared */

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

    /*! Set Chi-squared
    */
    void setChi(int Chi) { m_Chi = Chi; }


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

    /*! Get Chi-squared
     * @return Chi-squared
     */
    int getChi() const { return m_Chi; }


  private:

    int m_CellId;      /**< Cell ID */
    int m_Amp;         /**< Fitting Amplitude */
    int m_TimeFit;     /**< Fitting Time */
    int m_Quality;     /**< Fitting Quality */
    int m_Chi;    /**< Fitting chi2-squared */

    // 2: r25301, data member changed to ints
    ClassDef(ECLDigit, 2);/**< ClassDef */

  };
} // end namespace Belle2

