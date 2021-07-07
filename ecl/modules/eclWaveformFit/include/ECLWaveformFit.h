/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

// FRAMEWORK
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

// ECL
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLDsp.h>

class TMinuit;

namespace Belle2 {

  /** Struct to keep upper triangle of the covariance matrix. Since the
  *  matrix is already inverted we do not need extra precision, so
  *  keep matrix elements in float type to save space.
  *  sigma is the average noise.
  */
  struct CovariancePacked {
    /** packed matrix*/
    float m_covMatPacked[31 * (31 + 1) / 2] = {};
    /** sigma noise*/
    float sigma{ -1};
    /** lvalue access by index */
    float& operator[](int i) { return m_covMatPacked[i];}
    /** rvalue access by index */
    const float& operator[](int i) const { return m_covMatPacked[i];}
  };

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
    explicit SignalInterpolation2(const std::vector<double>&);
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
    StoreArray<ECLDsp> m_eclDSPs;  /**< StoreArray ECLDsp */
    StoreArray<ECLDigit> m_eclDigits;   /**< StoreArray ECLDigit */

    double m_EnergyThreshold{0.03};  /**< energy threshold to fit pulse offline*/
    double m_chi2Threshold25dof{57.1};  /**< chi2 threshold (25 dof) to classify offline fit as good fit.*/
    double m_chi2Threshold27dof{60.0};  /**< chi2 threshold (27 dof) to classify offline fit as good fit.*/
    bool m_TemplatesLoaded{false};  /**< Flag to indicate if waveform templates are loaded from database.*/
    void loadTemplateParameterArray();  /**< loads waveform templates from database.*/
    std::vector<double> m_ADCtoEnergy;  /**< calibration vector form adc to energy*/

    TMinuit* m_Minit2h{nullptr};   /**< minuit minimizer for optimized fit*/
    TMinuit* m_Minit2h2{nullptr};   /**< minuit minimizer for optimized fit with background photon*/
    void Fit2h(double& b, double& a0, double& t0, double& a1, double& chi2);  /**< Optimized fit using hadron component model*/
    void Fit2hExtraPhoton(double& b, double& a0, double& t0, double& a1, double& A2, double& T2,
                          double& chi2);  /**< Optimized fit using hadron component model plus out of time background photon*/
    SignalInterpolation2 m_si[8736][3];  /**< ShaperDSP signal shapes.*/

    CovariancePacked m_c[8736] = {};  /**< Packed covariance matrices */
    bool m_CovarianceMatrix{true};  /**< Option to use crystal dependent covariance matrices.*/
    bool m_IsMCFlag{false};  /**< Flag to indicate if running over data or MC.*/
  };
} // end Belle2 namespace
