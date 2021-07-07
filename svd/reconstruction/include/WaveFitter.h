/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef WAVEFITTER_H
#define WAVEFITTER_H

#include <array>
#include <vector>
#include <algorithm>
#include <functional>
#include <cmath>

namespace Belle2 {
  namespace SVD {

    /** A functor to provide a simple model of APV25 strip response.*/
    class DefaultWave {
    public:

      /** Constructor takes waveform decay time as parameter.
       * @param tau waveform decay time
       */
      DefaultWave(double tau = s_default_tau) : m_tau(tau) {}

      static double s_default_tau;  /**< Default waveform decay time */

      /** getValue() returns the value at desired time and time shift.
       * It is normalized to unit amplitude (value at maximum).
       * @param t time at which wave has to be calculated
       * @param t0 time shift - time when the wave starts to rise from 0
       * @return waveform at time t
       */
      inline double getValue(double t) const
      {
        if (t < 0)
          return 0.0;
        else {
          double z = t / m_tau;
          return z * std::exp(1.0 - z);
        }
      }

      /** return the decay time*/
      double getTau() const { return m_tau; }

    private:
      double m_tau; /**< Waveform decay time */
    };


    /** Waveform fitter class */
    class WaveFitter {
    public:

      /** Function type for waveform */
      typedef std::function<double(double)> wave_function_type;
      /** Function type for cluster data */
      typedef std::vector< std::array<double, 6> > strip_data_type;

      /** Constructor creates an empty fitter with default settings.
       * @param wave Wave function to use for fitting data
       * @param times array of points in time at which data are taken
       */
      WaveFitter(wave_function_type wave, std::array<double, 6> times):
        m_hasFit(false), m_times(times), m_wave(wave), m_fittedTime(0), m_fittedTimeError(), m_fittedLik(1.0e10), m_ndf(0)
      {
        m_dt = m_times[1] - m_times[0];
      }

      /** Add strip data to the fitter.
       * @param inpData array of 6 APV signals.
       * @param rmsNoise RMS noise for the strip.
       */
      void addData(const std::array<double, 6>& inpData, double rmsNoise)
      {
        std::array<double, 6> v;
        std::copy(inpData.begin(), inpData.end(), v.begin());
        m_data.push_back(v);
        m_noises.push_back(rmsNoise);
        m_hasFit = false;
        m_ndf = 6 * m_data.size() - m_data.size() - 2;
      }

      /** Reset fitter data. */
      void reset()
      {
        m_data.clear();
        m_noises.clear();
        m_fittedData.clear();
        m_fittedAmplitudes.clear();
        m_fittedAmplitudeErrors.clear();
        m_ndf = 0;
        m_hasFit = false;
      }

      /** Retrieve fitted time shift value after a fit
       * @return time shift estimate
       */
      double getFittedTime()
      {
        if (!m_hasFit) doFit();
        return m_fittedTime;
      }

      /** Retrieve error of time shift value after a fit.
       * @return time shift estimate error
       */
      double getFittedTimeError()
      {
        if (!m_hasFit) doFit();
        return m_fittedTimeError;
      }

      /** Retrieve strip amplitudes after a fit
       * @return vector<double> of strip amplitude estimates
       */
      const std::vector<double>& getFittedAmplitudes()
      {
        if (!m_hasFit) doFit();
        return m_fittedAmplitudes;
      }

      /** Retrieve errors of strip amplitudes after a fit
       * @return vector<double> of errors of strip amplitude estimates
       */
      const std::vector<double>& getFittedAmplitudeErrors()
      {
        if (!m_hasFit) doFit();
        return m_fittedAmplitudeErrors;
      }

      /** Return negative log-likelihood for the fit
       * @return negative log-likelihood for the fit
       */
      double getFitLikelihood()
      {
        if (!m_hasFit) doFit();
        return m_fittedLik;
      }

      /** Get original fitted data
       * @return vector of arrays of strip data used in the fit.
       */
      const strip_data_type& getData() const { return m_data; }

      /** Get fitted values for all samples
       * @return vector of arrays of fit estimates of fitted data.
       */
      const strip_data_type& getFitData()
      {
        if (!m_hasFit) doFit();
        return m_fittedData;
      }

      /** Get sampling interval */
      double getDt() const { return m_dt; }
      /** Get sample times */
      const std::array<double, 6>& getTimes() const { return m_times; }

      /** Get standardized chi2 for a given time shift
       * @param t time shift
       * @return chi2/ndf at a given time shift
       */
      double Chi(double t);
      /** Get negative profile log-likelihood for a given time shift
       * @param t the time shift at which to calculate the likelihood
       * @return log-likelihood integrated over amplitudes at the given t
       */
      double negLogLikelihood(double t);
      /** Calculate probability of the hit being a signal, with given priors for signal and background.
       */
      double pSignal();
      /** Calculate likelihood ratio Lsignal/Lbackground for acceptance (signal) window given as arguments
       * @param a lower bound of the acceptance window
       * @param b upper bound of the acceptance window
       * @return L(in acc. w.)/L(out of acc. w.)
       */
      double lrSignal(double a, double b);

      /** Get unit waveform value at a given time */
      double wave(double t) const { return m_wave(t); }
      /** Set precision of the minimzer */
      static void setPrecision(int ndigits);

    private:

      /**minimizer precision*/
      static int s_minimizer_precision;

      /** Perform fit on the data */
      void doFit();
      /** Calculate amplitudes.*/
      void calculateAmplitudes();
      /** Calculate fit errors.
       * This only calculates local (linearization) errors.
       */
      void calculateFitErrors();
      /** Calculate fitted data. */
      void calculateFittedData();
      /** High-order Gauss-Legendre quadrature for likelihood integrals */
      // cppcheck-suppress unusedPrivateFunction
      double integral12(double lower, double upper, std::function<double(double)> f);
      /** High-order Gauss-Legendre quadrature for likelihood integrals */
      double integral20(double lower, double upper, std::function<double(double)> f);
      bool m_hasFit;    /**< Are fit results available? */
      double m_dt;  /**< Time interval between samples */
      strip_data_type m_data;   /**< Vector of sextets of APV samples */
      std::vector<double> m_noises; /**< RMS noise for strips */
      std::array<double, 6> m_times; /**< Vector of sample times */
      wave_function_type m_wave;  /**< Waveform function */
      double m_fittedTime; /**< Time from interval (-dt,+dt). */
      double m_fittedTimeError; /**< Error estimate of the fitted time. */
      std::vector<double> m_fittedAmplitudes; /**< Fitted amplitudes of the sample. */
      std::vector<double> m_fittedAmplitudeErrors;  /**< Fitted amplitude errors. */
      double m_fittedLik; /**< Chi2 from the current fit. */
      int m_ndf; /**< Degrees of freedom of the fit */
      strip_data_type m_fittedData; /**< Fitted values from current fit. */
    };

  } // namespace SVD
} // namespace Belle2
#endif // WAVEFITTER_H
