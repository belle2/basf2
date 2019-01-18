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
#include "TObject.h"

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
    SVDLocalConfigParameters()
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
     * @return float corresponding to time units [Accelerator RFC]
     */
    float getTimeUnits(void) const { return m_timeUnits; };

    /**
     * Returns the masking bitmap used to mask the strips at FADC level (only for CM-section)
     * @param none
     * @return int corresponding to the mask filter applied at FADC level
     */
    int getMaskFilter(void) const { return m_maskFilter; };

    /**
     * Returns the time stamp date_hour (yyyymmdd_hhmm) of the current calibration     * @param none
     * @return string corresponding to the time stamp of the current calibration in the format yyyymmdd_hhmm.
     */
    std::string getCalibDate(void) const { return m_calibDate; };

    /**
     * Set the injected charge
     * Input:
     * @param float
     *
     */
    void setInjectedCharge(float injectedCharge)
    {
      m_injectedCharge = injectedCharge;
    }

    /**
     * Set the time units
     * Input:
     * @param float
     *
     */
    void setTimeUnits(float timeUnits)
    {
      m_timeUnits = timeUnits;
    }

    /**
     * Set the masking bitmap
     * Input:
     * @param int
     *
     */
    void setMaskFilter(int maskFilter)
    {
      m_maskFilter = maskFilter;
    }

    /**
     * Set the date_hour (yyyymmdd_hhmm) of the current calibration
     * Input:
     * @param std::string
     *
     */
    void setCalibDate(std::string date)
    {
      m_calibDate = date;
    }
  private:



    /** charge in electrons injected in each strip to calibrate the pulse gain during the calibration run
     */
    float m_injectedCharge;

    /** Time units of the measured pulse shape peak time expressed in accelerator RFC
     */
    float m_timeUnits;

    /** masking bitmap
     */
    int m_maskFilter;

    /** time stamp with date and hour(yyyymmdd_hhmm) of when the local runs for the current calibration have been taken
     */
    std::string m_calibDate;

    ClassDef(SVDLocalConfigParameters, 1);

  };

}
