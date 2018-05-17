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

  struct val_der_t {
    double f0, f1, f2;
  };

  struct SignalInterpolation2 {
    constexpr static int c_nt = 12, c_ndt = 5, c_ntail = 20;
    constexpr static double c_dt = 0.5, c_idt = 1 / c_dt, c_dtn = c_dt / c_ndt, c_idtn = c_ndt / c_dt;
    std::pair<double, double> m_F[c_nt * c_ndt + c_ntail];
    double m_r0, m_r1;
    SignalInterpolation2() {};
    SignalInterpolation2(const std::vector<double>&);
    void getshape(double, val_der_t*) const;
    val_der_t operator()(double) const;
  };

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

    virtual const char* eclDigitArrayName() const
    { return "ECLDigits" ; }

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
