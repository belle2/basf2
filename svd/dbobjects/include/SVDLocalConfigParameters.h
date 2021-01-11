/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Laura Zani, Eugenio Paoloni                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 * WARNING, ALBEIT INTENSE COHERENT LIGHT IS NOT USED AT ALL HERE:        *
 * PLEASE, PLEASE, PLEASE WEAR ALWAYS PROTECTIVE GLASSES                  *
 * WHEN LOOKING AT THIS FILE                                              *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <TString.h>

namespace Belle2 {
  /**
   * This class defines the payload which stores to the central DB the
   * SVD local configuring parameters, and the methods to import and retrieve
   * these parameters.
   *
   */
  class SVDLocalConfigParameters : public TObject {
  public:

    /**
    * Default constructor
    */
    SVDLocalConfigParameters(const TString& uniqueID = "")
      : m_uniqueID(uniqueID),
        m_injectedCharge(22500)
    {};
    /**
     * Returns the injected charge during the calibration run
     * @param none
     * @return float corresponding to injected charge
     */
    float getInjectedCharge(void) const { return m_injectedCharge; };

    /**
     * Returns the time units in which the peak time of the pulse
     * shape is provided.
     * @param none
     * @return float corresponding to calibration time units [Accelerator RFC converted in ns]
     */
    float getCalibrationTimeInRFCUnits(void) const
    {
      TString aux(m_calibrationTimeUnits);
      aux = aux.Remove(aux.First(" "), aux.Sizeof());

      std::string auxString(aux);
      int calibrationTimeCoeff = std::atoi(auxString.c_str());
      return calibrationTimeCoeff;
    }

    /**
     * Returns the time stamp date_hour (yyyymmdd_hhmm) of the current calibration     * @param none
     * @return string corresponding to the time stamp of the current calibration in the format yyyymmdd_hhmm.
     */
    std::string getCalibDate(void) const { return m_calibDate; };

    /**
     * Set the injected charge
     * Input:
     * @param injectedCharge
     *
     */
    void setInjectedCharge(float injectedCharge)
    {
      m_injectedCharge = injectedCharge;
    }


    /**
     * Set the time units
     * Input:
     * @param calibrationTimeUnits
     *
     */
    void setCalibrationTimeInRFCUnits(const std::string& calibrationTimeUnits)
    {
      m_calibrationTimeUnits = calibrationTimeUnits;
    }

    /**
     * Set the date (yyyy-mm-dd) of the current NOISE calibration
     * Input:
     * @param date
     *
     */
    void setCalibDate(const std::string& date)
    {
      m_calibDate = date;
    }


    /**
     * Get the unique ID  of the calibration
     */
    TString get_uniqueID() const {return m_uniqueID;}

  private:

    TString m_uniqueID; /**<The unique identifier is a private member of SVDLocalConfigParameter, whose value is assigned in the constructor.*/


    /** charge in electrons injected in each strip to calibrate the pulse gain during the calibration run
     */
    float m_injectedCharge;


    /** Time units of the measured pulse shape peak time expressed in accelerator RFC
     */
    std::string m_calibrationTimeUnits;

    /** time stamp with date and hour(yyyymmdd_hhmm) of when the local runs for the current calibration have been taken
     */
    std::string m_calibDate;

    ClassDef(SVDLocalConfigParameters, 1); /**<needed by root*/

  };

}
