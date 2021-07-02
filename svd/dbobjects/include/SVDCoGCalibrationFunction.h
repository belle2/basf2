/**************************************************************************
\ * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Eugenio Paoloni, Giulia Casarosa                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>

#include <cmath>
#include <vector>

namespace Belle2 {

  /** class to contain the CoG Time calibrations*/
  class SVDCoGCalibrationFunction : public TObject {

  public:

    /** typedef of the return value of the calibration function*/
    typedef double (SVDCoGCalibrationFunction::*cogFunction)(double, int) const;

    /** typedef of the return value of the calibration function ERROR*/
    typedef double (SVDCoGCalibrationFunction::*cogFunctionErr)(double, double, int) const;

    /** returns the calibrated value of raw_time, depending on the trigger bin*/
    double calibratedValue(double raw_time, int trigger_bin) const
    {
      cogFunction f = m_implementations[m_current];
      return (this->*f)(raw_time, trigger_bin) ;
    }
    /** returns the error of the calibrated value of raw_time, depending on the trigger bin*/
    double calibratedValueError(double raw_time, double raw_timeErr, int trigger_bin) const
    {
      cogFunctionErr fErr = m_implementationsErr[m_current];
      return (this->*fErr)(raw_time, raw_timeErr, trigger_bin) ;
    }

    /** constructor */
    SVDCoGCalibrationFunction()
    {
      // The m_implementations vector is static.
      // We have to initialize it just once.
      if (m_implementations.size() == 0) {
        m_implementations.push_back(&SVDCoGCalibrationFunction::pol1TBdep);
        m_implementations.push_back(&SVDCoGCalibrationFunction::pol3TBindep);
        m_implementations.push_back(&SVDCoGCalibrationFunction::pol5TBindep);
        m_implementations.push_back(&SVDCoGCalibrationFunction::elsTBindep);
        //m_implementations.push_back(
        //  &SVDCoGCalibrationFunction::betterVersion);
      }

      // The m_implementationsErr vector is static.
      // We have to initialize it just once.
      if (m_implementationsErr.size() == 0) {
        m_implementationsErr.push_back(&SVDCoGCalibrationFunction::pol1TBdepErr);
        m_implementationsErr.push_back(&SVDCoGCalibrationFunction::pol3TBindepErr);
        m_implementationsErr.push_back(&SVDCoGCalibrationFunction::pol5TBindepErr);
        m_implementationsErr.push_back(&SVDCoGCalibrationFunction::elsTBindepErr);
        //m_implementationsErr.push_back(
        //  &SVDCoGCalibrationFunction::betterVersion);
      }

      m_current = m_implementations.size() - 1;

    };

    /** allows to choose the function version */
    void set_current(int current)
    {
      m_current = current;
    }

    //SETTERS FOR function ID = 0 (pol1TBdep)
    /** set the trigger bin dependent shift*/
    void set_bias(double tb0, double tb1, double tb2, double tb3)
    {
      m_bias[0] = tb0;
      m_bias[1] = tb1;
      m_bias[2] = tb2;
      m_bias[3] = tb3;
    }
    /** set the trigger bin dependent scale*/
    void set_scale(double tb0, double tb1, double tb2, double tb3)
    {
      m_scale[0] = tb0;
      m_scale[1] = tb1;
      m_scale[2] = tb2;
      m_scale[3] = tb3;
    }


    //SETTERS FOR function ID = 1 (pol3TBindep)
    /** set the pol3 TB independent parameters*/
    void set_pol3parameters(double a, double b, double c, double d)
    {
      m_par[ 0 ] = a;
      m_par[ 1 ] = b;
      m_par[ 2 ] = c;
      m_par[ 3 ] = d;
    }

    //SETTERS FOR function ID = 2 (pol5TBindep)
    /** set the pol5 TB independent parameters*/
    void set_pol5parameters(double a, double b, double c, double d, double e, double f)
    {
      m_par[ 0 ] = a;
      m_par[ 1 ] = b;
      m_par[ 2 ] = c;
      m_par[ 3 ] = d;
      m_par[ 4 ] = e;
      m_par[ 5 ] = f;
    }

    //SETTERS FOR function ID = 3 (elsTBindep)
    /** set the parameters for the ELS TB independent function*/
    void set_elsparameters(double a, double b, double c, double d)
    {
      m_par[ 0 ] = a;
      m_par[ 1 ] = b;
      m_par[ 2 ] = c;
      m_par[ 3 ] = d;
    }

    /** copy constructor */
    SVDCoGCalibrationFunction(const Belle2::SVDCoGCalibrationFunction& a);

    /** operator = */
    SVDCoGCalibrationFunction& operator=(const Belle2::SVDCoGCalibrationFunction& a);

  private:

    /** total number of trigger bins */
    static const int nTriggerBins = 4;

    /** function parameters & implementations*/

    /** ID = {0}, pol1TBdep VERSION: correctedValue = t * scale[tb] + bias[tb] */
    double m_bias[ nTriggerBins ] = {0}; /**< trigger-bin dependent bias*/
    double m_scale[ nTriggerBins ] = {0}; /**< trigger-bin dependent scale*/
    /** pol1 TB dep version implementation*/
    double pol1TBdep(double raw_time, int tb) const
    {
      return raw_time * m_scale[ tb % nTriggerBins] +
             m_bias[ tb % nTriggerBins ];
    };
    /** implementation of pol1 TB dep error*/
    double pol1TBdepErr(double , double raw_timeErr, int tb) const
    {
      return raw_timeErr * m_scale[ tb % nTriggerBins];
    };

    //data member useful for polinomials
    static const int m_nPar = 6; /**< number of parameters of highest-order implemented pol (5)*/
    double m_par[ m_nPar ] = {0}; /**< vector of parameters*/

    /** ID = 1, pol3TBindep VERSION: (TB independent) correctedValue = par[0] + t * par[1] + t^2 * par[2] + t^3 * par[3] */
    /** pol3 TB indep version implementation*/
    double pol3TBindep(double raw_time, int) const
    {
      return m_par[0] + m_par[1] * raw_time + m_par[2] * pow(raw_time, 2) + m_par[3] * pow(raw_time, 3);
    };
    /** implementation of pol3 TB indep error*/
    double pol3TBindepErr(double raw_time, double raw_timeErr, int) const
    {
      return raw_timeErr * (m_par[1] +  2 * m_par[2] * raw_time + 3 * m_par[3] * pow(raw_time, 2));
    };


    /** ID = 2, pol5TBindep VERSION: (TB independent) correctedValue = par[0] + t * par[1] + t^2 * par[2] + t^3 * par[3] + t^4 * par[4] + t^5*par[5]*/
    /** pol5 TB indep version implementation*/
    double pol5TBindep(double raw_time, int) const
    {
      return m_par[0] + m_par[1] * raw_time + m_par[2] * pow(raw_time, 2) + m_par[3] * pow(raw_time, 3) + m_par[4] * pow(raw_time,
             4) + m_par[5] * pow(raw_time, 5);
    };
    /** implementation of pol5 TB indep error*/
    double pol5TBindepErr(double raw_time, double raw_timeErr, int) const
    {
      return raw_timeErr * (m_par[1] +  2 * m_par[2] * raw_time + 3 * m_par[3] * pow(raw_time, 2) + 4 * m_par[4] * pow(raw_time,
                            3) + 5 * m_par[5] * pow(raw_time, 4));
    };

    /** ID = 3, elsTBindep VERSION: (TB independent)
    correctedValue = par[0] + t * par[1] + par[2]/(t - par[3])  if t<=par[3]-sqrt(-par[2])/4
    correctedValue = NaN otherwise
    */
    /** els TB indep version implementation*/
    double elsTBindep(double raw_time, int) const
    {
      if (raw_time > m_par[3] - sqrt(-m_par[2]) / 4)
        return  std::numeric_limits<float>::quiet_NaN();

      return m_par[0] + m_par[1] * raw_time + m_par[2] / (raw_time - m_par[3]);
    };

    /** implementation of els TB indep error*/
    double elsTBindepErr(double raw_time, double raw_timeErr, int) const
    {
      if (raw_time > m_par[3] - sqrt(-m_par[2]) / 4)
        return  std::numeric_limits<float>::quiet_NaN();

      return raw_timeErr * (m_par[1] - m_par[2] / pow(raw_time - m_par[3], 2));
    };

    /** current function ID */
    int m_current;

    /** vector of fuctions for time calibration, we use the m_current*/
    static std::vector < cogFunction > m_implementations; //! Do not stream this, please throw it in the WC

    /** vector of fuctions for time error calibration, we use the m_current*/
    static std::vector < cogFunctionErr > m_implementationsErr; //! Do not stream this, please throw it in the WC


    ClassDef(SVDCoGCalibrationFunction, 6)
  };

}
