/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#ifndef _SVD_RECONSTRUCTION_NNWAVEFITTOOL_H
#define _SVD_RECONSTRUCTION_NNWAVEFITTOOL_H

#include <cmath>
#include <vector>
#include <map>
#include <memory>
#include <tuple>
#include <algorithm>
#include <numeric>
#include <functional>
#include <sstream>
#include <framework/logging/Logger.h>
#include <svd/simulation/SVDSimulationTools.h>

namespace Belle2 {
  namespace SVD {

    /** Vector of values defined for bins, such as bin times or bin
     * probabilities */
    typedef std::vector<double> nnFitterBinData;

    /** Vector of bin edges, nnFitterBinData.size() + 1 */
    typedef std::vector<double> nnFitterBins;

    /** Empirical distribution function object is basic for mainpulation
     * of probabilities.
     */

    class EmpiricalDistributionFunction {
    public:
      /** Constructor constructs edf object from a probability distribution.
       * @param p array of bin probabilities
       * @param bins array of bin boundaries, size(p) + 1
       */
      EmpiricalDistributionFunction(const nnFitterBinData& p, const nnFitterBins& bins):
        m_bins(bins)
      {
        // Create table of probabiilities for EmpiricalDistributionFunction calculation
        m_pTable.resize(m_bins.size());
        auto ibin = m_bins.begin();
        auto ipt = m_pTable.begin();
        double cp = 0.0;
        *ipt++ = std::make_pair(*ibin++, cp);
        for (auto prob : p) {
          cp += prob;
          *ipt++ = std::make_pair(*ibin++, cp);
        }
      }

      /** EmpiricalDistributionFunction(t) gives edf value at time t, linearly interpolated from cummulative
       * bin probabilities.
       * @param t time at which edf is to be calculated.
       */
      double operator()(double t)
      {
        // pTable is sorted by .first
        if (t > m_pTable.back().first) return 1.0;
        if (t < m_pTable[0].first) return 0.0;
        auto it = lower_bound(m_pTable.begin(), m_pTable.end(), std::make_pair(t, 0.0));
        if (it == m_pTable.begin()) return it->second;
        auto it2 = it;
        --it2;
        double result =  it2->second + (it->second - it2->second) * (t - it2->first) / (it->first - it2->first);
        return result;
      }

    private:
      const nnFitterBins& m_bins; /**< array of bin boundaries. */
      std::vector<std::pair<double, double> > m_pTable; /**< (bin, prob) pairs */

    }; // class EmpiricalDistributionFunction

    // ========================================================================
    // NNWaveFitTool - object to handle computations with neural network fit.
    // ------------------------------------------------------------------------
    /** The class holds arrays of bins and bin centers, and a wave generator object
     * containing information on the waveform function. It can compute probability
    * distribution updates in clustering, or the parameters of waveform fit, such as the
    * time shift, amplitude and their errors, and it can do compatibility tests.
    */
    class NNWaveFitTool {
    public:
      /** Constructor takes arrays of signals and bin probabilities.
       * @param bins Pointer to array of time bin edges
       * @param binCenters Pointer to array of mean bin times
       * @param wave Wave generator to generate waveform samples.
       */
      NNWaveFitTool(const nnFitterBins& bins, const nnFitterBinData& binCenters,
                    const WaveGenerator& wave):
        m_bins(bins), m_binCenters(binCenters), m_waveGenerator(wave)
      {
        m_altBinData.resize(m_binCenters.size());
      }

      //------- Operations on probability arrays -------//

      /** Get edges of time shift bins
       * @return pointer to array of bin edges
       */
      const nnFitterBins& getBins() const { return m_bins; }

      /** Get mean bin time shifts.
       * @return pointer to array of bin time shifts
       */
      const nnFitterBinData& getBinCenters() const { return m_binCenters; }

      /** Shift the probability array in time.
       * The method works by linearly interpolating the EmpiricalDistributionFunction
       * to calculate values for a shifted set of bins.
       * @param p the probability array to be modified.
       * @param timeShift the size of the shift.
       */
      void shiftInTime(nnFitterBinData& p, double timeShift);

      /** Multiply probabilities.
       * Modify first probability distribution by multiplying it with
       * the second.
       */
      void multiply(nnFitterBinData& p, const nnFitterBinData& p1)
      {
        std::transform(p.begin(), p.end(),
                       p1.begin(), p.begin(), std::multiplies<double>());
        normalize(p);
      }

      /** Convert a uniform distribution to time-shift-like pdf.
       * @parram left Left edge of the interval
       * @param right Right edge of the interval
       * @return pointer to binned pdf representing the uniform distribution
       * between left and right.
       */
      std::shared_ptr<nnFitterBinData> pFromInterval(double left, double right);

      //------- Waveform parameters ------------------------------------//

      /** Return std::tuple containing time shift and its error.
       * @param p array of bin probabilities
       * @return std::tuple of timeShift, timeShiftError
       */
      std::tuple<double, double> getTimeShift(const nnFitterBinData& p);

      /** Return std::tuple with signal amplitude, its error, and chi2 of the fit.
       * The amplitude is calculated using linear regression through origin, its error as the
       * error of the regression coefficient.
       * @param samples array of 6 samples to fit
       * @param timeShift fitted time shift
       * @param tau width of the waveform
       * @return std::tuple of amplitude, its error, and chi2
       */
      std::tuple<double, double, double>
      getAmplitudeChi2(const apvSamples& samples, double timeShift, double tau);

      /** Return the probability X < Y, where X and Y are random variables with binned pdfs
       * p1 and p2.
       * @param p1 binned pdf for X
       * @param p2 binned pdf for Y
       */
      double pLessThan(nnFitterBinData p1, nnFitterBinData p2);

    protected:

      /** Normalize the probability distribution.
       * After shifting, multiplications or other ooperations on probabilities,
       * we need to restore the normalization and re-build the list used for
       * calculation of EmpiricalDistributionFunction.
       * If probabilities vanish over all bins, return uniform distribution.
       */
      void normalize(nnFitterBinData& p)
      {
        double pnorm = std::accumulate(p.begin(), p.end(), 0.0);
        // If the norm is too small, return default distribution.
        if (pnorm < 1.0e-10) {
          double uniformP = 1.0 / std::distance(p.begin(), p.end());
          std::fill(p.begin(), p.end(), uniformP);
        } else {
          // Normalize if the norm makes sense.
          std::transform(p.begin(), p.end(), p.begin(),
                         std::bind2nd(std::divides<double>(), pnorm));
        }
      }

    private:
      const nnFitterBins& m_bins; /**< Edges of bins */
      const nnFitterBinData& m_binCenters; /**< Centers of bins */
      nnFitterBinData m_altBinData; /**< Bin data array for re-use. */
      apvSamples m_altSamples; /**< Array of 6 apv samples for re-use. */
      WaveGenerator m_waveGenerator; /**< APV signal generator */
    }; // class NNWaveFitTool

  } // SVD namespace
} // Belle2 namespace

#endif
