/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* ECL headers. */
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/dbobjects/ECLAutoCovariance.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <ecl/dbobjects/ECLDigitWaveformParameters.h>
#include <ecl/dbobjects/ECLDigitWaveformParametersForMC.h>

/* Basf2 headers. */
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>

class TMinuit;

namespace Belle2 {

  /**
   * Struct to keep upper triangle of the covariance matrix. Since the
   * matrix is already inverted we do not need extra precision, so
   * keep matrix elements in float type to save space.
   * sigma is the average noise.
   */
  struct CovariancePacked {

    /** Packed matrix. */
    float m_covMatPacked[31 * (31 + 1) / 2] = {};

    /** Sigma noise. */
    float sigma{ -1};

    /** Lvalue access by index. */
    float& operator[](int i) { return m_covMatPacked[i];}

    /** Rvalue access by index. */
    const float& operator[](int i) const { return m_covMatPacked[i];}
  };

  /**
   * Interpolation of signal shape using function values and
   * the first derivative.
   */
  struct SignalInterpolation2 {

    /**
     * Signal function is sampled in c_nt time steps with c_ndt substeps
     * and c_ntail steps. c_dt is the time step.
     */
    constexpr static int c_nt = 12;

    /** Number of substeps. */
    constexpr static int c_ndt = 5;

    /** Number of tail steps. */
    constexpr static int c_ntail = 20;

    /** Time step. */
    constexpr static double c_dt = 0.5;

    /** Inverted time step */
    constexpr static double c_idt = 1 / c_dt;

    /** Time substep. */
    constexpr static double c_dtn = c_dt / c_ndt;

    /** Inverted time substep. */
    constexpr static double c_idtn = c_ndt / c_dt;

    /** Function values. */
    double m_FunctionInterpolation[c_nt * c_ndt + c_ntail];

    /** Derivative values. */
    double m_DerivativeInterpolation[c_nt * c_ndt + c_ntail];

    /**
     * Assuming exponential drop of the signal function far away from 0,
     * extrapolate it to +inf.
     * f(i_last + i) = f(i_last)*m_r0^i
     * f'(i_last + i) = f'(i_last)*m_r1^i
     * where i_last is the last point within sampled values in
     * m_FunctionInterpolation (m_DerivativeInterpolation).
     */
    double m_r0;

    /** See above/ */
    double m_r1;

    /**
     * Default constructor.
     */
    SignalInterpolation2() {};

    /**
     * Constructor with parameters with the parameter layout as
     * in ECLDigitWaveformParameters.
     */
    explicit SignalInterpolation2(const std::vector<double>&);

    /**
     * Returns signal shape and derivatives in 31 equidistant time points
     * starting from t0.
     * @param[in]  t0          Time.
     * @param[out] function    Function values.
     * @param[out] derivatives Derivatives.
     */
    void getShape(double t0, double* function, double* derivatives) const;

  };

  /**
   * Module performs offline fit for saved ECL waveforms.
   */
  class ECLWaveformFitModule : public Module {

  public:

    /** Constructor.
     */
    ECLWaveformFitModule();

    /** Destructor.
     */
    ~ECLWaveformFitModule();

    /** Initialize variables. */
    virtual void initialize() override;

    /** begin run.*/
    virtual void beginRun() override;

    /** event per event.
     */
    virtual void event() override;

    /** end run. */
    virtual void endRun() override;

    /** terminate.*/
    virtual void terminate() override;

    /** ECLDigits Array Name.*/
    virtual const char* eclDigitArrayName() const
    { return "ECLDigits" ; }

    /** ECLDspsArray Name.*/
    virtual const char* eclDspArrayName() const
    { return "ECLDsps" ; }


  private:

    /**
     * Loads waveform templates from database.
     */
    void loadTemplateParameterArray();

    /**
     * Optimized fit using hadron component model.
     */
    void Fit2h(double& b, double& a0, double& t0, double& a1, double& chi2);

    /**
     * Optimized fit using hadron component model plus out-of-time
     * background photon.
     */
    void Fit2hExtraPhoton(double& b, double& a0, double& t0, double& a1,
                          double& A2, double& T2, double& chi2);

    /** Energy threshold to fit pulse offline. */
    double m_EnergyThreshold{0.03};

    /** chi2 threshold (25 dof) to classify offline fit as good fit. */
    double m_chi2Threshold25dof{57.1};

    /** chi2 threshold (27 dof) to classify offline fit as good fit. */
    double m_chi2Threshold27dof{60.0};

    /** Flag to indicate if waveform templates are loaded from database. */
    bool m_TemplatesLoaded{false};

    /** Calibration vector from ADC to energy. */
    std::vector<double> m_ADCtoEnergy;

    /** Minuit minimizer for optimized fit. */
    TMinuit* m_Minit2h{nullptr};

    /** Minuit minimizer for optimized fit with background photon. */
    TMinuit* m_Minit2h2{nullptr};

    /** ShaperDSP signal shapes. */
    SignalInterpolation2 m_si[ECLElementNumbers::c_NCrystals][3];

    /** Packed covariance matrices. */
    CovariancePacked m_c[ECLElementNumbers::c_NCrystals] = {};

    /** Option to use crystal dependent covariance matrices. */
    bool m_CovarianceMatrix{true};

    /** Flag to indicate if running over data or MC. */
    bool m_IsMCFlag{false};

    /** Crystal electronics. */
    DBObjPtr<ECLCrystalCalib> m_CrystalElectronics{"ECLCrystalElectronics"};

    /** Crystal energy. */
    DBObjPtr<ECLCrystalCalib> m_CrystalEnergy{"ECLCrystalEnergy"};

    /** Waveform parameters. */
    DBObjPtr<ECLDigitWaveformParameters> m_WaveformParameters;

    /** Waveform parameters for MC. */
    DBObjPtr<ECLDigitWaveformParametersForMC> m_WaveformParametersForMC;

    /** Autocovariance. */
    DBObjPtr<ECLAutoCovariance> m_AutoCovariance;

    /** StoreArray ECLDsp. */
    StoreArray<ECLDsp> m_eclDSPs;

    /** StoreArray ECLDigit. */
    StoreArray<ECLDigit> m_eclDigits;

  };

}
