/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#ifndef _SVD_SIMULATION_SIMULATIONTOOLS_H
#define _SVD_SIMULATION_SIMULATIONTOOLS_H

#include <cmath>
#include <array>
#include <algorithm>
#include <functional>
#include <sstream>
#include <framework/gearbox/Unit.h>

namespace Belle2 {
  namespace SVD {

// ==============================================================================
// APV25 sample data
// ------------------------------------------------------------------------------
// Constants:
    /** Number of APV samples */
    const std::size_t nAPVSamples = 6;

    /** APV sampling time */
    const double dt_APV = 31.44 * Unit::ns;

    /** Vector of input samples in float form */
    typedef double apvSampleBaseType;
    typedef std::array<apvSampleBaseType, nAPVSamples> apvSamples; /**< vector od apvSample BaseType objects*/

    /** APV time base - times for the 6 signals */
    const apvSamples apvTimeBase =
    {{ -dt_APV, 0.0, dt_APV, 2 * dt_APV, 3 * dt_APV, 4 * dt_APV}};

// ==============================================================================
// APV25 waveform shapes
// ------------------------------------------------------------------------------

    /** WaveformShape type.
     * This is the type for a naked waveform function, giving a single value for a
     * properly scaled arguemnt.
     * The functions are scaled to 1.0 at mode, location (shift) 0.0, and scale 1.0.
     */
    typedef std::function<double(double)> WaveformShape;

    /** Gamma waveform shape, x.exp(-x)
     * This is only historically useful. Use beta-prime instead.
     * @param t properly scaled time, (t-t0)/tau
     * @return Waveform value t * exp(1 - t) for t > 0, else 0
     */
    inline double w_expo(double t)
    {
      if (t < 0.0) return 0.0;
      else return t * exp(1.0 - t);
    }

    /** Polynomial waveform shape, x.(1-x)^2
     * This is used in signal calibration in SVD hardware tests.
     * Do not use for signal simulation.
     * @param t properly scaled time, (t-t0)/tau
     * @return Waveform value 27/4 * t * (1 - t)^2 for t in <0,1>, else 0
     */
    inline double w_poly3(double t)
    {
      if (t < 0.0 || t > 1.0)
        return 0.0;
      else
        return 6.75 * t * (1.0 - t) * (1.0 - t);
    }

    /** Beta-prime waveform shape, x^alpha/(1+x)^beta.
     * This is the function for general use.
     * @param t Properly scaled time, (t - t0)/tau.
     * @return Waveform value 149.012 * t^2 / (1+t)^10 for t > 0, else 0
     */
    inline double w_betaprime(double t)
    {
      if (t < 0.0)
        return 0.0;
      else
        return 149.012 * pow(t, 2) * pow(1.0 + t, -10);
    }

    /** Adjacent-channel waveform U-side.
     * The exact parametrisation is to be determined.
     * @param t Properly scaled time, (t - t0)/tau.
     * @return Waveform value (TBD).
     */
    inline double w_adjacentU(double t)
    {

      double p[8];

      // waveform parameters
      p[0] = -7.379;
      p[1] = -0.3594;
      p[2] =  0.07564;
      p[3] =  7.391;
      p[4] = -0.08004;
      p[5] = -2.139e-5;
      p[6] = -1.781e-7;
      p[7] =  0.01827;

      if (t < 0.0)
        return 0.0;
      else
        return (p[0] + p[1] * t) * exp(-p[2] * t) + (p[3] + p[4] * t + p[5] * pow(t, 2) + p[6] * pow(t, 3)) * exp(-p[7] * t);
    }

    /** Adjacent-channel waveform V-side.
     * The exact parametrisation is to be determined.
     * @param t Properly scaled time, (t - t0)/tau.
     * @return Waveform value (TBD).
     */
    inline double w_adjacentV(double t)
    {

      double p[8];

      // waveform parameters
      p[0] =  0.2043;
      p[1] = -0.0197;
      p[2] =  0.01448;
      p[3] = -0.2076;
      p[4] =  0.0387;
      p[5] =  0.01055;
      p[6] = -4.03e-6;
      p[7] =  0.07193;

      if (t < 0.0)
        return 0.0;
      else
        return (p[0] + p[1] * t) * exp(-p[2] * t) + (p[3] + p[4] * t + p[5] * pow(t, 2) + p[6] * pow(t, 3)) * exp(-p[7] * t);
    }


    /** Waveform generator
     * This is a functor to calculate APV samples from waveform.
     * Constructs from a waveform function.
     */

    class WaveGenerator {
    public:
      /** Constructor takes waveform function. */
      WaveGenerator(WaveformShape waveform = w_betaprime):
        m_samples( {{0, 0, 0, 0, 0, 0}}), m_waveform(waveform)
      {}
      /** Set waveform */
      void setWaveform(WaveformShape waveform) { m_waveform = waveform; }
      /** Operator () returns 6 APV samples.*/
      const apvSamples& operator()(double t0, double tau)
      {
        std::transform(
          apvTimeBase.begin(), apvTimeBase.end(),
          m_samples.begin(),
          [this, t0, tau](double t)->double { return m_waveform((t - t0) / tau); }
        );
        return m_samples;
      }
    private:
      apvSamples m_samples; /**< for storage of computed data */
      WaveformShape m_waveform; /**< the wave function */
    };


    // ==============================================================================
    // The 3-samples filter
    // ------------------------------------------------------------------------------

    /** pass zero suppression*/
    template<typename T>
    inline void zeroSuppress(T& a, double thr)
    {
      std::replace_if(a.begin(), a.end(), std::bind2nd(std::less<double>(), thr), 0.0);
    }

// ==============================================================================
// The 3-samples filter
// ------------------------------------------------------------------------------

    /** pass 3-samples*/
    template<typename T>
    inline bool pass3Samples(const T& a, double thr)
    {
      return (search_n(a.begin(), a.end(), 3, thr, std::greater<double>()) != a.end());
    }

// ==============================================================================
// Tau (scale) conversion and encoding
// ------------------------------------------------------------------------------

    /** Convert Hao's raw tau (integral, in latency units) to correct betaprime
     * scale. Includes scaling and fit adjustmenta
     */
    inline double tau_raw2real(double raw_tau) { return 7.32313 * raw_tau; }

    /** Encoder/decoder for neural network tau values.
     * Tau value is an additional input to the network along with the 6 signal samples.
     * Therefore, it is scaled to the range of signals, more precisely, to the amplitude
     * range.
     */
    class TauEncoder {
    public:
      /** Constructor takes amplitude and tau ranges, typically from a network xml file.
       * @param min_amplitude Lower bound of the amplitude range
       * @param max_amplitude Upper bound of the amplitude range
       * @param min_tau Lower bound of the tau range
       * @param max_tau Upper bound of the tau range
       */
      TauEncoder(double min_amplitude, double max_amplitude,
                 double min_tau, double max_tau):
        m_minAmplitude(min_amplitude), m_maxAmplitude(max_amplitude),
        m_minTau(min_tau), m_maxTau(max_tau)
      {
        m_ATRatio = (m_maxAmplitude - m_minAmplitude) / (m_maxTau - m_minTau);
      }
      /** Default constructor
       * Parameterless constructor to default-construct the class. */
      TauEncoder(): TauEncoder(0, 100, 0, 100) {}

      /** Set encoder baounds (e.g. when default constructor was used)
       * @param min_amplitude Lower bound of the amplitude range
       * @param max_amplitude Upper bound of the amplitude range
       * @param min_tau Lower bound of the tau range
       * @param max_tau Upper bound of the tau range
       */
      void setBounds(double min_amplitude, double max_amplitude,
                     double min_tau, double max_tau)
      {
        m_minAmplitude = min_amplitude;
        m_maxAmplitude = max_amplitude;
        m_minTau = min_tau;
        m_maxTau = max_tau;
        // Re-initialize the range ratio setting
        m_ATRatio = (m_maxAmplitude - m_minAmplitude) / (m_maxTau - m_minTau);
      }

      /** Return encoded value for a waveform width tau value.
       * @param tau original tau value
       * @return tau value scaled to amplitude range
       */
      double encodeTau(double tau) const
      { return m_minAmplitude + m_ATRatio * (tau - m_minTau); }

      /** Scale back a previously scaled tau value.
       * @param  scaledTau a previously scaled tau value
       * @return restored tau value
       */
      double decodeTau(double scaledTau) const
      { return m_minTau + 1.0 / m_ATRatio * (scaledTau - m_minAmplitude); }

      /**print tool*/
      void print(std::ostringstream& os) const
      {
        os << "TauEncoder object, "
           << "Amplitude range: " << m_minAmplitude << " to " << m_maxAmplitude
           << " Tau range: " << m_minTau << " to " << m_maxTau
           << " ATRatio: " << m_ATRatio << std::endl;
      }

    private:
      double m_minAmplitude; /**< lower bound of amplitude range */
      double m_maxAmplitude; /**< upper bound of amplitdue range */
      double m_minTau; /**< lower bound of tau range */
      double m_maxTau; /**< upper bound of tau range */
      double m_ATRatio; /**< ratio of amplitude to tau ranges */
    };
// ------------------------------------------------------------------------------

  } // namespace SVD
} // namespace Belle2

#endif
