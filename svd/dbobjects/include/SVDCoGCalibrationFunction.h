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

#include <TObject.h>
#include <functional>
#include <vector>
#include <framework/logging/Logger.h>
namespace Belle2 {

  class SVDCoGCalibrationFunction : public TObject {

  public:


    typedef double (SVDCoGCalibrationFunction::*cogFunction)(double, int) const;

    /** returns the calibrated value of raw_time, depending on the trigger bin*/
    double calibratedValue(double raw_time, int trigger_bin)
    {
      cogFunction f = m_implementations[m_current];
      return (this->*f)(raw_time, trigger_bin) ;
    }
    /** returns the error of the calibrated value of raw_time, depending on the trigger bin*/
    double calibratedValueError(int trigger_bin)
    {
      return m_residSigma[trigger_bin % nTriggerBins];
    }

    /** constructor */
    SVDCoGCalibrationFunction(double bias = 0., double scale = 1., double residMean = 0., double residSigma = 1., double error = 0)
    {
      for (int i = 0; i < nTriggerBins; i++) {
        m_bias[i] = bias;
        m_scale[i] = scale;
        m_scaleError[i] = error;
        m_biasError[i] = error;
        m_scaleBiasCovariance[i] = error;

        m_residMean[i] = residMean;
        m_residSigma[i] = residSigma;
        m_residMeanError[i] = error;
        m_residSigmaError[i] = error;
      }

      // The m_implementations vector is static.
      // We have to initialize it just once.
      if (m_implementations.size() == 0) {
        m_implementations.push_back(&SVDCoGCalibrationFunction::firstVersion);
        //m_implementations.push_back(
        //  &SVDCoGCalibrationFunction::betterVersion);
      }

      m_current = m_implementations.size() - 1;

    };


    void set_bias(double tb0, double tb1, double tb2, double tb3)
    {
      m_bias[0] = tb0;
      m_bias[1] = tb1;
      m_bias[2] = tb2;
      m_bias[3] = tb3;
    }
    void set_scale(double tb0, double tb1, double tb2, double tb3)
    {
      m_scale[0] = tb0;
      m_scale[1] = tb1;
      m_scale[2] = tb2;
      m_scale[3] = tb3;
    }

    void set_biasError(double tb0, double tb1, double tb2, double tb3)
    {
      m_biasError[0] = tb0;
      m_biasError[1] = tb1;
      m_biasError[2] = tb2;
      m_biasError[3] = tb3;
    }
    void set_scaleError(double tb0, double tb1, double tb2, double tb3)
    {
      m_scaleError[0] = tb0;
      m_scaleError[1] = tb1;
      m_scaleError[2] = tb2;
      m_scaleError[3] = tb3;
    }
    void set_scaleBiasCovariance(double tb0, double tb1, double tb2, double tb3)
    {
      m_scaleBiasCovariance[0] = tb0;
      m_scaleBiasCovariance[1] = tb1;
      m_scaleBiasCovariance[2] = tb2;
      m_scaleBiasCovariance[3] = tb3;
    }


    /** copy constructor */
    SVDCoGCalibrationFunction(const Belle2::SVDCoGCalibrationFunction& a);


  private:

    /** total number of trigger bins */
    static const int nTriggerBins = 4;

    /** residual = (EventT0 - CoGTime Corrected) mean and sigma from gaussian fit*/
    double m_residMean[ nTriggerBins ]; /**< trigger-bin dependent residual mean*/
    double m_residSigma[ nTriggerBins ]; /**< trigger-bin dependent residual sigma*/
    double m_residMeanError[ nTriggerBins ]; /**< trigger-bin dependent mean error*/
    double m_residSigmaError[ nTriggerBins ]; /**< trigger-bin dependent sigma error*/


    /** function parameters & implementations*/

    /** FIRST VERSION: correctedValue = t * scale[tb] + bias[tb] */
    double m_bias[ nTriggerBins ]; /**< trigger-bin dependent bias*/
    double m_scale[ nTriggerBins ]; /**< trigger-bin dependent scale*/
    double m_biasError[ nTriggerBins ]; /**< trigger-bin dependent bias error*/
    double m_scaleError[ nTriggerBins ]; /**< trigger-bin dependent scale error*/
    double m_scaleBiasCovariance[ nTriggerBins ]; /**< trigger-bin dependent covariance between scale and bias errors*/
    /** first version implementation*/
    double firstVersion(double raw_time, int tb) const
    {
      return raw_time * m_scale[ tb % nTriggerBins] +
             m_bias[ tb % nTriggerBins ];
    };

    /** current function ID */
    int m_current;
    /** vector of fuctions, we use the m_current*/
    static std::vector < cogFunction > m_implementations; //! Do not stream this, please throw it in the WC


    ClassDef(SVDCoGCalibrationFunction, 1)
  };

}
