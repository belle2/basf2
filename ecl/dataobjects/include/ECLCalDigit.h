/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <ecl/dataobjects/ECLDsp.h>

namespace Belle2 {

  /*! Class to store calibrated ECLDigits: ECLCalDigits
   */

  class ECLCalDigit : public RelationsObject {
  public:

    /** status enumerator */
    enum StatusBit {
      c_IsEnergyCalibrated = 1 << 0,
      c_IsTimeCalibrated = 1 << 1,
      c_IsTimeResolutionCalibrated = 1 << 2,
      c_IsFailedFit = 1 << 3,
      c_IsFailedTimeResolution = 1 << 4,
      c_IsCalibrated = c_IsEnergyCalibrated | c_IsTimeCalibrated | c_IsTimeResolutionCalibrated,
    };

    /** default constructor for ROOT */
    ECLCalDigit()
    {
      m_CellId         = 0; /**< Cell ID */
      m_Time           = 0; /**< Calibrated Time */
      m_TimeResolution = 0; /**< Calibrated Time Resolution */
      m_Energy         = 0; /**< Calibrated Energy */
      m_Status         = 0; /**< Calibration Status */
      m_TwoComponentTotalEnergy = 0; /**< Offline Two Component Total Energy*/
      m_TwoComponentHadronEnergy = 0; /**< Offline Two Component Hadron Energy*/
      m_TwoComponentDiodeEnergy = 0; /**< Offline Two Component Diode Energy*/
      m_TwoComponentChi2 = -1; /**< Offline Two Component chi2*/
      m_TwoComponentSavedChi2[0] = -1;  /**< Offline two component chi2 FT=0*/
      m_TwoComponentSavedChi2[1] = -1;  /**< Offline two component chi2 FT=1*/
      m_TwoComponentSavedChi2[2] = -1;  /**< Offline two component chi2 FT=2*/
      m_TwoComponentFitType = ECLDsp::poorChi2; /**< Offline Two Component fit type*/
    }

    /*! Set  Cell ID
     */
    void setCellId(int CellId) { m_CellId = CellId; }

    /*! Set Calibrated Energy
     */
    void setEnergy(double Energy) { m_Energy = Energy; }

    /*! Set two component total energy
     */
    void setTwoComponentTotalEnergy(double Energy) { m_TwoComponentTotalEnergy = Energy; }

    /*! Set two component hadron energy
     */
    void setTwoComponentHadronEnergy(double Energy) { m_TwoComponentHadronEnergy = Energy; }

    /*! Set two component diode energy
     */
    void setTwoComponentDiodeEnergy(double Energy) { m_TwoComponentDiodeEnergy = Energy; }

    /*! Set two component chi2
     */
    void setTwoComponentChi2(double chi) { m_TwoComponentChi2 = chi; }

    /*! Set two comp chi2 for a fit type
     *  see enum TwoComponentFitType in ECLDsp.h for description of fit types.
     */
    void setTwoComponentSavedChi2(ECLDsp::TwoComponentFitType FitTypeIn, double input)
    {
      unsigned int index = FitTypeIn ;
      m_TwoComponentSavedChi2[index] = input;
    }

    /*! Set two component fit type
     */
    void setTwoComponentFitType(ECLDsp::TwoComponentFitType ft) { m_TwoComponentFitType = ft; }

    /*! Set Calibrated Time
     */
    void setTime(double Time) { m_Time = Time; }

    /*! Set Calibrated Time Resolution
     */
    void setTimeResolution(double TimeResolution) { m_TimeResolution = TimeResolution; }

    /*! Set Calibration Status (overwrites previously set bits)
     */
    void setStatus(unsigned short int status) { m_Status = status; }

    /*! Add Calibration Status
     */
    void addStatus(unsigned short int bitmask) { m_Status |= bitmask; }

    /*! Remove Calibration Status
     */
    void removeStatus(unsigned short int bitmask) { m_Status &= (~bitmask); }

    /*! Get Cell ID
     * @return cell ID
     */
    int getCellId() const { return m_CellId; }

    /*! Get Calibrated Energy
     * @return Calibrated Energy
     */
    double getEnergy() const { return m_Energy; }

    /*! Get Two Component calibrated Total Energy
     * @return Two Component calibrated Total Energy
     */
    double getTwoComponentTotalEnergy() const { return m_TwoComponentTotalEnergy; }

    /*! Get Two Component calibrated hadron component Energy
     * @return Two Component calibrated hadron component Energy
     */
    double getTwoComponentHadronEnergy() const { return m_TwoComponentHadronEnergy; }

    /*! Get Two Component calibrated diode component Energy
     * @return Two Component calibrated diode component Energy
     */
    double getTwoComponentDiodeEnergy() const { return m_TwoComponentDiodeEnergy; }

    /*! Get two componnent chi2
     * @return two componnent chi2
     */
    double getTwoComponentChi2() const { return m_TwoComponentChi2; }

    /*! get two comp chi2 for a fit type
     *  see enum TwoComponentFitType in ECLDsp.h for description of fit types.
     * @return two comp chi2 for fit type
     */
    double getTwoComponentSavedChi2(ECLDsp::TwoComponentFitType FitTypeIn) const
    {
      unsigned int index = FitTypeIn ;
      return m_TwoComponentSavedChi2[index];
    }

    /*! Get two componnent fit type
     * @return two componnent fit type
     */
    ECLDsp::TwoComponentFitType getTwoComponentFitType() const { return m_TwoComponentFitType; }

    /*! Get Calibrated Time
     * @return Calibrated Time
     */
    double getTime() const { return m_Time; }

    /*! Get Calibrated Time Resolution
     * @return Calibrated Time Resolution
     */
    double getTimeResolution() const { return m_TimeResolution; }

    /*! Get Calibration Status
     * @return Calibration Status
     */
    bool hasStatus(unsigned short int bitmask) const { return (m_Status & bitmask) == bitmask; }

    /*! Get Boolean Calibration Status
    * @return Calibration Status
    */
    bool isCalibrated() const;

    /*! Get Boolean Energy Calibration Status
    * @return Energy Calibration Status
    */
    bool isEnergyCalibrated() const;

    /*! Get Boolean Time Calibration Status
     * @return Time Calibration Status
     */
    bool isTimeCalibrated() const;

    /*! Get Boolean Time Resolution Calibration Status
     * @return Time Resolution Calibration Status
     */
    bool isTimeResolutionCalibrated() const;

    /*! Get Boolean Fit Failed Status
     * @return Fit Failed Status
     */
    bool isFailedFit() const;

    /*! Get Boolean time resolution failed status
     * @return time resolution failed
     */
    bool isTimeResolutionFailed() const;

  private:

    int m_CellId;   /**< Cell ID */
    double m_Time;   /**< Calibrated Time */
    double m_TimeResolution;   /**< Calibrated Time resolution*/
    double m_Energy; /**< Calibrated Energy */
    unsigned short int m_Status;   /**< Calibration and Fit Status */
    double m_TwoComponentTotalEnergy;  /**< Calibrated Two Component Total Energy */
    double m_TwoComponentHadronEnergy; /**< Calibrated Hadron Component Energy */
    double m_TwoComponentDiodeEnergy; /**< Calibrated Diode Component Energy */
    double m_TwoComponentChi2; /**< Two Component chi2*/
    double m_TwoComponentSavedChi2[3]; /**< Two comp chi2 for each fit tried in reconstruction */
    ECLDsp::TwoComponentFitType m_TwoComponentFitType;  /**< offline fit hypothesis.*/

    // 1: first version (TF)
    // 2: added m_TimeResolution (TF)
    // 3: added status bits for failed fits (TF)
    // 4: added offline fit variables (SL)
    // 5: added diode and pile-up photon offline fit hypothesis (SL)
    // 6: added m_TwoComponentSavedChi2[3] to save chi2 for each fit tried (SL)
    ClassDef(ECLCalDigit, 6); /**< ClassDef */

  };

  // inline isCalibrated
  inline bool ECLCalDigit::isCalibrated() const
  {
    return hasStatus(c_IsCalibrated);
  }

  // inline isEnergyCalibrated
  inline bool ECLCalDigit::isEnergyCalibrated() const
  {
    return hasStatus(c_IsEnergyCalibrated);
  }

  // inline isTimeCalibrated
  inline bool ECLCalDigit::isTimeCalibrated() const
  {
    return hasStatus(c_IsTimeCalibrated);
  }

  // inline isTimeResolutionCalibrated
  inline bool ECLCalDigit::isTimeResolutionCalibrated() const
  {
    return hasStatus(c_IsTimeResolutionCalibrated);
  }

  // inline isFailedFit
  inline bool ECLCalDigit::isFailedFit() const
  {
    return hasStatus(c_IsFailedFit);
  }

  // inline isTimeResolutionFailed
  inline bool ECLCalDigit::isTimeResolutionFailed() const
  {
    return hasStatus(c_IsFailedTimeResolution);
  }


} // end namespace Belle2
