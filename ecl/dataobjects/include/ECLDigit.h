/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <ecl/utility/ECLRawDataHadron.h>

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
      m_Chi = 0;       /**< Fitting chisquared */
    }

    /*                              Setters                                   */

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
    void setQuality(unsigned int NewQuality)
    {
      //                             4    0
      //                             v    v
      const unsigned int mask = 0b0000'0011;
      // Set quality in bits 1..0, keep other bits unchanged
      m_Quality = (NewQuality & mask) | (m_Quality & ~mask);
    }

    /*! Set Chi-squared
    */
    void setChi(int Chi) { m_Chi = Chi; }

    /** Set hadron fraction
     * See ecl/utility/include/ECLRawDataHadron.h for details.
     */
    void setPackedHadronFraction(unsigned int packed_fraction)
    {
      //                             8    4    0
      //                             v    v    v
      const unsigned int mask = 0b0011'1110'0000;
      // Set packed hadron fraction in bits 9..5, keep other bits unchanged
      m_Quality = ((packed_fraction << 5) & mask) | (m_Quality & ~mask);
    }

    /** Set ShaperDSP data format.
     * See the table in eclUnpackerModule.cc for details
     */
    void setDataFormat(unsigned int format)
    {
      //                             4    0
      //                             v    v
      const unsigned int mask = 0b0001'1100;
      // Set data availability flag in bits 4..2, keep other bits unchanged
      m_Quality = ((format << 2) & mask) | (m_Quality & ~mask);
    }

    /*                              Getters                                   */

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
    int getQuality() const
    {
      //                             4    0
      //                             v    v
      const unsigned int mask = 0b0000'0011;
      // Get quality from bits 1..0
      return m_Quality & mask;
    }

    /*! Get Chi-squared
     * @return Chi-squared
     */
    int getChi() const { return m_Chi; }

    /** Get packed hadron fraction
     * This is a raw value from the ShaperDSP firmware, useful only for debugging
     */
    unsigned int getPackedHadronFraction() const
    {
      //                             8    4    0
      //                             v    v    v
      const unsigned int mask = 0b0011'1110'0000;
      // Get packed hadron fraction from bits 9..5
      return (m_Quality & mask) >> 5;
    }

    /** Get unpacked hadron fraction
     * This is the value of (A_hadron / A_total) from ShaperDSP firmware that
     * supports hadron component fit.
     * For other firmware versions it is set to 0.
     */
    double getHadronFraction() const
    {
      using namespace Belle2::ECL::RawDataHadron;
      unsigned int packed_fraction = getPackedHadronFraction();
      // Unpack from [0..31] to a floating point value. See unpackHadronFraction for details
      return unpackHadronFraction(packed_fraction);
    }

    /** Get ShaperDSP data format.
     * See the table in eclUnpackerModule.cc for details
     */
    unsigned int getDataFormat()
    {
      //                             4    0
      //                             v    v
      const unsigned int mask = 0b0001'1100;
      return (m_Quality & mask) >> 2;
    }

    /**
     * Find ECLDigit by Cell ID using linear search.
     * This is shown to be faster than getRelated(..) method.
     */
    static ECLDigit* getByCellID(int cid);

  private:

    int m_CellId;            /**< Cell ID */
    int m_Amp;               /**< Fitting Amplitude */
    int m_TimeFit;           /**< Fitting Time */
    /**
     * Fit information
     *  - Fit quality (0-good, 1-int overflow/underflow, 2-low amplitude, 3-bad chi^2)
     *  - Output data format (see eclUnpackerModule.cc for details)
     *  - Packed hadron component fraction for versions of the ShaperDSP firmware that support hadron component fit.
     */
    int m_Quality;
    int m_Chi;               /**< Fitting chi2-squared */

    // 2: r25301, data member changed to ints
    ClassDef(ECLDigit, 2);/**< ClassDef */

  };

} // end namespace Belle2

