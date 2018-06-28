/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Eugenio Paoloni                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <functional>
#include <vector>
#include <framework/logging/Logger.h>
namespace Belle2 {

  class SVDCoGCalibrationFunction {

  public:


    typedef double (SVDCoGCalibrationFunction::*cogFunction)(double, int) const;

    /** returns the calibrated value of raw_time, depending on the trigger bin*/
    double calibratedValue(double raw_time, int trigger_bin)
    {
      cogFunction f = m_implementations[m_current];
      B2INFO("PAYLOAD, calibratedValue: shift  = " << m_shift[0] << " scale = " << m_scale[0]);
      return (this->*f)(raw_time, trigger_bin) ;
    }

    /** constructor */
    SVDCoGCalibrationFunction()
    {

      m_implementations.push_back(&SVDCoGCalibrationFunction::zeroVersion);
      m_implementations.push_back(&SVDCoGCalibrationFunction::firstVersion);
      m_shift[0] = 42.42;
      m_scale[0] = 24.24;
      m_current = m_implementations.size() - 1;
    };

    SVDCoGCalibrationFunction(const SVDCoGCalibrationFunction& a);

    SVDCoGCalibrationFunction operator = (const Belle2::SVDCoGCalibrationFunction& a) { return SVDCoGCalibrationFunction(a) ; };

    void set_shift(double tb0, double tb1, double tb2, double tb3)
    {
      m_shift[0] = tb0;
      m_shift[1] = tb1;
      m_shift[2] = tb2;
      m_shift[3] = tb3;
      B2INFO("shift TB 0 = " << m_shift[0]);
    }

    void set_scale(double tb0, double tb1, double tb2, double tb3)
    {
      m_scale[0] = tb0;
      m_scale[1] = tb1;
      m_scale[2] = tb2;
      m_scale[3] = tb3;
      B2INFO("scale TB 0 = " << m_scale[0]);
    }

    void print_par() {B2INFO("stored shift = " << m_shift[0] << ", stored scale = " << m_scale[0]);}

  private:

    /** total number of trigger bins */
    const int nTriggerBins = 4;

    /** function parameters & implementations*/

    /** ZERO VERSION: correctedValue = t + shift */
    double m_shiftZero; /**< trigger-bin independent shift*/
    double zeroVersion(double raw_time, int) const {return raw_time + m_shiftZero; };

    /** FIRST VERSION: correctedValue = t * scale[tb] + shift[tb] */
    double m_shift[4]; /**< trigger-bin dependent shift*/
    double m_scale[4]; /**< trigger-bin dependent scale*/
    /** first version implementation*/
    double firstVersion(double raw_time, int tb) const {return raw_time * m_scale[tb % 4] + m_shift[tb % 4]; };


    /** current function ID */
    int m_current;
    /** vector of fuctions, we use the m_current*/
    static std::vector < cogFunction > m_implementations;


  };

}
