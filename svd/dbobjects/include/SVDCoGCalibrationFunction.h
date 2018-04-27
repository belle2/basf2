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

namespace Belle2 {

  class SVDCoGCalibrationFunction {

  public:


    typedef double (SVDCoGCalibrationFunction::*cogFunction)(double, int) const;

    /** returns the calibrated value of raw_time, depending on the trigger bin*/
    double calibratedValue(double raw_time, int trigger_bin)
    {
      cogFunction f = m_implementations[m_current];
      return (this->*f)(raw_time, trigger_bin) ;
    }

    /** constructor */
    SVDCoGCalibrationFunction()
    {

      m_implementations.push_back(&SVDCoGCalibrationFunction::zeroVersion);
      m_implementations.push_back(&SVDCoGCalibrationFunction::firstVersion);

      m_current = m_implementations.size() - 1;
    };

  private:

    /** total number of trigger bins */
    const int nTriggerBins = 4;

    /** function parameters & implementations*/

    /** ZERO VERSION: correctedValue = t + bias */
    double m_biasZero; /**< trigger-bin independent bias*/
    double zeroVersion(double raw_time, int) const {return raw_time + m_biasZero; };

    /** FIRST VERSION: correctedValue = t * scale[tb] + bias[tb] */
    double m_bias[4]; /**< trigger-bin dependent bias*/
    double m_scale[4]; /**< trigger-bin dependent scale*/
    /** first version implementation*/
    double firstVersion(double raw_time, int tb) const {return raw_time * m_scale[tb] + m_bias[tb]; };


    /** current function ID */
    int m_current;
    /** vector of fuctions, we use the m_current*/
    static std::vector < cogFunction > m_implementations;


  };

}
