/********************************************************************
 * BASF2 (Belle Analysis Framework 2)                               *
 * Copyright(C) 2018 - Belle II Collaboration                       *
 *                                                                  *
 * Author: The Belle II Collaboration                               *
 * Contributors: Savino Longo (longos@uvic.ca)                      *
 *               Alexei Sibidanov (sibid@uvic.ca)                   *
 *                                                                  *
 * This software is provided "as is" without any warranty.          *
 ********************************************************************/

#pragma once

// FRAMEWORK
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/database/DBObjPtr.h>

// ECL
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/dbobjects/ECLDigitWaveformParameters.h>
#include <ecl/digitization/shaperdsp.h>

class TMinuit;

namespace Belle2 {

  /** Struct to return signal function information
   * f0 is the function value
   * f1 is the first derivative
   * f2 is the second derivative
   */
  struct val_der_t {
    /** see struct description */
    double f0;
    /** see struct description */
    double f1;
    /** see struct description */
    double f2;
  };

  /** Interpolate signal shape using function values and the first derivative.
   */
  struct SignalInterpolation2 {
    /**
    * Signal function is sampled in c_nt time steps with c_ndt substeps + c_ntail steps
    * c_dt is the time step.
    */
    constexpr static int c_nt = 12;
    /** substeps */
    constexpr static int c_ndt = 5;
    /** tail steps */
    constexpr static int c_ntail = 20;
    /** time step */
    constexpr static double c_dt = 0.5;
    /** inverted time step */
    constexpr static double c_idt = 1 / c_dt;
    /** time substep */
    constexpr static double c_dtn = c_dt / c_ndt;
    /** inverted time substep */
    constexpr static double c_idtn = c_ndt / c_dt;
    /**
     * storage for function value + first derivative
     */
    std::pair<double, double> m_F[c_nt * c_ndt + c_ntail];
    /**
     * assuming exponential drop of the signal function far away from 0, extrapolate it to +inf
     * f(i_last + i) = f(i_last)*m_r0^i
     * f'(i_last + i) = f'(i_last)*m_r1^i
     * where i_last is the last point within sampled values in m_F
     */
    double m_r0;
    /** see above */
    double m_r1;

    /** Default constructor. */
    SignalInterpolation2() {};
    /** Constructor with parameters with the parameter layout as in ECLDigitWaveformParameters*/
    SignalInterpolation2(const std::vector<double>&);
    /**
     *  returns signal shape(+derivatives) in 31 equidistant time points
     *  starting from T0
     */
    void getshape(double, val_der_t*) const;
  };

  /** Module performs offline fit for saved ecl waveforms.
   *    */
  class ECLWaveformFitModule : public Module {

  public:

    /** Constructor.
     */
    ECLWaveformFitModule();

    /** Destructor.
     */
    ~ECLWaveformFitModule();

    /** Initialize variables. */
    virtual void initialize();

    /** begin run.*/
    virtual void beginRun();

    /** event per event.
     */
    virtual void event();

    /** end run. */
    virtual void endRun();

    /** terminate.*/
    virtual void terminate();

    /** ECLDigits Array Name.*/
    virtual const char* eclDigitArrayName() const
    { return "ECLDigits" ; }

    /** ECLDspsArray Name.*/
    virtual const char* eclDspArrayName() const
    { return "ECLDsps" ; }


  private:
    StoreArray<ECLDsp> m_eclDSPs;  /** StoreArray ECLDsp*/
    StoreArray<ECLDigit> m_eclDigits;   /** StoreArray ECLDigit*/

    double m_EnergyThreshold;  /**energy threshold to fit pulse offline*/
    double m_TriggerThreshold;  /**energy threshold for waveform trigger.*/
    bool m_TemplatesLoaded;  /**Flag to indicate if waveform templates are loaded from database.*/
    void loadTemplateParameterArray(bool IsDataFlag);  /** loads waveform templates from database.*/
    std::vector<double> m_ADCtoEnergy;  /**calibration vector form adc to energy*/

    TMinuit* m_Minit2h;   /** minuit minimizer for optimized fit*/
    void Fit2h(double& b, double& a0, double& t0, double& a1, double& chi2);  /** Optimized fit using hadron component model*/
    SignalInterpolation2 m_si[8736][3];  /**ShaperDSP signal shapes.*/
  };
} // end Belle2 namespace
