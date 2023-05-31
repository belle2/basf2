/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/logging/Logger.h>

#include <cmath>
#include <iostream>
#include <TObject.h>
#include <TCanvas.h>
#include <TH1F.h>

namespace Belle2 {

  /**
   *   dE/dx injection time calibration constants
   */

  class CDCDedxInjectionTime: public TObject {

  public:

    /**
     * Default constructor
     */
    CDCDedxInjectionTime(): m_injectionvar() {};

    /**
     * Constructor
     */
    CDCDedxInjectionTime(const std::vector<std::vector<double>>& vinjcalib): m_injectionvar(vinjcalib) {};

    /**
     * Destructor
     */
    ~CDCDedxInjectionTime() {};


    /** Return dE/dx mean or norm-reso value for the given time and ring
     * @param svar is option for printing mean and reso calibration
     * @param sfx to add suffix in file save
     */
    void printCorrection(std::string svar, std::string sfx) const;


    /** Return vector of all constant vector of payload
     *
     */
    std::vector<std::vector<double>> getConstVector() const
    {
      return m_injectionvar;
    };

    /** Return time vector
     * @param ring is injection ring number (0/1 for LER/HER)
     */
    std::vector<double> getTimeVector(const unsigned int ring) const
    {
      if (ring > 2) B2ERROR("wrong index for injection ring ");
      return m_injectionvar[ring * 3];
    };

    /** Return dedx mean vector
     * @param ring is injection ring number (0/1 for LER/HER)
     */
    std::vector<double> getMeanVector(const int ring) const
    {
      if (ring <= 0 || ring > 2) B2ERROR("wrong index for injection ring ");
      return m_injectionvar[ring * 3 + 1];
    };

    /** Return dedx reso vector
     * @param ring is injection ring number (0/1 for LER/HER)
     */
    std::vector<double> getResoVector(const unsigned int ring) const
    {
      if (ring > 2) B2ERROR("wrong index for injection ring ");
      return m_injectionvar[ring * 3 + 2];
    };

    /** Return time bin for the given time array
    * @param array of time
    * @param value of input time
    */
    unsigned int getTimeBin(const std::vector<unsigned int>& array, unsigned int value) const
    {
      int nabove, nbelow, middle;
      nabove = array.size() + 1;
      nbelow = 0;
      while (nabove - nbelow > 1) {
        middle = (nabove + nbelow) / 2;
        if (value == array[middle - 1]) return middle - 1;
        if (value  < array[middle - 1]) nabove = middle;
        else nbelow = middle;
      }
      return nbelow - 1;
    }

    /** Return dE/dx mean or norm-reso value for the given time and ring
    * @param svar is option for mean and reso calibration factor
    * @param ring is injection ring number (0/1 for LER/HER)
    * @param time is injection time (large 0-20sec range)
    */
    double getCorrection(std::string svar, unsigned int ring, unsigned int time) const;


  private:

    /** CDC dE/dx injection time payloads for dEdx mean and reso.
     * different for LER and HER
    */
    std::vector<std::vector<double>> m_injectionvar; /**< vector to store payload values*/
    ClassDef(CDCDedxInjectionTime, 1); /**< ClassDef */

  };
} // end namespace Belle2
