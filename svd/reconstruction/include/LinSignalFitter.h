/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef LinSignalFitter_H
#define LinSignalFitter_H

#include <vector>
#include <map>
#include <utility>
#include <math.h>

namespace Belle2 {
  namespace SVD {

    /** LinSignalFitter - a toy waveform fitter.
     * The class is based on a specific representation of the signal waveform as a
     * gamma-type distribution. This allows to treat the fit as a linear regression
     * with non-linear mapping of parameters.
     * The fitter takes as input a series of signal samples taken on a single strip,
     * the sampling time, and the decay time. The decay time is not fitted,
     * and it is expected that fixed values of the decay time for electrons and
     * holes will work at realistic noise levels.
     */

    class LinSignalFitter {

    public:
      /** (time, signal) pair of a single waveform.*/
      typedef std::map<double, double> signal_type;

      /** Enum giving fit result types.*/
      enum fit_result {
        no_fit = 0,   /** no fit was tried yet */
        good_fit = 1, /** successful fit - use getters to get the result.*/
        bad_fit = 2   /** unsuccessful fit */
      };

      /** Constructor.
       * @param tau The decay time of the signal waveform.
       * @param samplingTime Sampling time.
       */
      LinSignalFitter(double tau = 50, double samplingTime = 30):
        m_tau(tau), m_samplingTime(samplingTime), m_hasFit(no_fit),
        m_initTime(0), m_initTimeSigma(0), m_charge(0), m_chargeSigma(0)
      {}

      /** Add a signal sample to the data.
       * @param time Time of the sample.
       * @param signal Recorded signal.
       */
      void setPoint(double time, double signal)
      { m_data.insert(std::make_pair(time, signal)); }

      /** Assign data to the map.
       * @param data map<double, double> of (time,signal) pairs.
       */
      void SetData(const signal_type& data)
      { m_data = data; }

      /** Waveform shape.
       * @param time The time at which the function is to be calculated.
       * @param initTime The initial time of the waveform.
       * @param charge The total charge of the waveform.
       * @return Value of the waveform at time time.
       */
      double waveform(double time, double initTime, double charge) const {
        if ((m_tau <= 0.0) || (time < initTime)) return 0;
        double z = (time - initTime) / m_tau; return charge * z * exp(1.0 - z);
      }

      /** Perform waveform fit.
       * @result fit_result type, 1 for successful fit, 2 for failure (charge set to 0).
       */
      int doFit();

      /** Get the number of signal samples.
       * @return The number of signal samples.
       */
      int getNSamples() const { return m_data.size(); }
      /** Get the estimated start time of the fitted waveform.
       * @return start time of the waveform.
       */
      double getInitTime() const { return m_initTime; }
      /** Get the error of the start time estimate.
       * @return standard deviation of the start time estimate.
       */
      double getInitTimeSigma() const { return m_initTimeSigma; }
      /** Get the strip signal estimate.
       * @return strip signal estimate.
       */
      double getCharge() const { return m_charge; }
      /** Get the standard deviation of strip signal estimate.
       * @return The standard deviations of signal estimate.
       */
      double getChargeSigma() const { return m_chargeSigma; }
      /** Get the original data.
       * @return map<strip,samples> of input signals.
       */
      const signal_type& getData() const { return m_data; }
      /** Get fit predictions.
       * @return signals predicted by the fit.
       */
      const signal_type& getPredictions() const { return m_predictions; }


    private:
      /** Waveform time constant. */
      double m_tau;
      /** Sampling time (interval between samples. */
      double m_samplingTime;
      /** Times and signals. */
      signal_type m_data;
      /** Values predicted by the fit. */
      signal_type m_predictions;
      /** Indicates whether a fit was already calculated or succeeded.*/
      fit_result m_hasFit;
      /** Initial time estimate of the waveform.*/
      double m_initTime;
      /** Initial time error estimate.*/
      double m_initTimeSigma;
      /** Strip charge estimate. */
      double m_charge;
      /** Charge error estimate. */
      double m_chargeSigma;
    };

  } // namespace SVD
} // namespace Belle2

#endif
