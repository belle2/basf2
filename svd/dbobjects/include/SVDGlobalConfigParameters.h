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
   * SVD global configuring parameters, and the methods to import and retrieve
   * these parameters.
   *
   */

  class SVDGlobalConfigParameters: public TObject {
  public:
    /**
    * Default constructor
    */
    SVDGlobalConfigParameters(const TString& uniqueID = "")
      : m_uniqueID(uniqueID),
        m_zeroSuppression(3),
        m_latency(0),
        m_maskFilter(7)
    {};


    /**
     * Returns the zero suppression threshold for global data taking in
     * ZS acquisition mode
     *
     * @param none
     * @return float corresponding to zero suppression cut applied during
     * data taking
     */
    float getZeroSuppression(void) const { return m_zeroSuppression; };

    /**
     * Returns the latency set for global data taking
     *
     * @param none
     * @return int corresponding to the latency set for global data taking
     */
    int getLatency(void) const { return m_latency; };

    /**
     * Returns the masking bitmap used to mask the strips at FADC level (only for CM-section)
     * @param none
     * @return int corresponding to the mask filter applied at FADC level
     */
    int getMaskFilter(void) const { return m_maskFilter; };

    /**
     * Returns the masking bitmap used to mask the strips at FADC level (only for CM-section)
     * @param none
     * @return flaot corresponding to the APV clock units in [ns]
     */
    float getAPVClockInRFCUnits(void) const
    {

      TString aux(m_APVClockInRFCUnits);
      aux = aux.Remove(aux.First(" "), aux.Sizeof());
      //    std::cout << "aux = " << aux <<endl;
      std::string auxString(aux);
      int APVClockUnitsCoeff = std::atoi(auxString.c_str());
      //    std::cout<<"time units coefficient = " << timeUnitsCoeff<<endl;

      return APVClockUnitsCoeff;
    }



    /**
     * Set the zero suppression
     * Input:
     * @param float
     *
     */
    void setZeroSuppression(float zeroSuppression)
    {
      m_zeroSuppression = zeroSuppression;
    }

    /**
     * Set the latency
     * Input:
     * @param int
     *
     */
    void setLatency(float latency)
    {
      m_latency = latency;
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
     * Set the APV Clock units
     * Input:
     * @param std::string coeff + units [RFC]
     *
     */
    void setAPVClockInRFCUnits(const std::string& APVClockUnits)
    {
      m_APVClockInRFCUnits = APVClockUnits;
    }


    /**
     * Get the unique ID  of the calibration
     */
    TString get_uniqueID() const {return m_uniqueID;}

  private:

    TString m_uniqueID;   /**< Add a string as unique identifier for a given local run configuration)*/

    /** zero suppression value which defines the S/N threshold of data mode acquisition during global run in ZS mode
         */
    float m_zeroSuppression;

    /** integer number of buffer cells to go back in the buffer ring to correctly sampling the signal pulse when the trigger is received.
     */
    int m_latency;

    /** masking bitmap
     */
    int m_maskFilter;

    /** APVclock
     */
    std::string m_APVClockInRFCUnits;

    ClassDef(SVDGlobalConfigParameters, 1); /**< needed by root*/

  };

}
