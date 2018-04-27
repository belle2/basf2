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

  class SVDCOGCalibrationFunction {

  public:

    /** returns the calibrated value of raw_time, depending on the trigger bin*/
    double calibratedValue(double raw_time, int trigger_bin)
    { return m_implemetations[m_current](raw_time, trigger_bin); }

    /** constructor */
    SVDCOOGCalibrationFunction()
    {
      m_implemetations.push(firstVersion);
      //      m_implemetations.push( secondVersion );

      m_current = m_implemetations.size() - 1;
    };

  private:

    /** total number of trigger bins */
    const int nTriggerBins = 4;

    /** function parameters & implementationt*/

    /** ZERO VERSION: correctedValue = t + bias */
    double m_biasZero; /**< trigger-bin independent bias*/
    double zeroVersion(double raw_time, int) {return raw_time + m_biasZero; };

    /** FIRST VERSION: correctedValue = t * scale[tb] + bias[tb] */
    double m_bias[4]; /**< trigger-bin dependent bias*/
    double m_scale[4]; /**< trigger-bin dependent scale*/
    /** first version implementation*/
    double firstVersion(double raw_time, int tb) {return raw_time * m_scale[tb] + m_bias[tb]; };

    //double secondVersion( double raw_time, int ) {return raw_time *m_scale + m_bias; };

    /** current function ID */
    int m_current;
    /** vector of fuctions, we use the m_current*/
    static std::vector < std::function< double(double, int) > m_implemetations;


  };

}
