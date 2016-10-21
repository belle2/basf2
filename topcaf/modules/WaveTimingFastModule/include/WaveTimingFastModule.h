/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Umberto Tamponi (tamponi@to.infn.it), Jan Strube,        *
 *               Bryan Fulson                                             *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/



#ifndef WaveTimingFastModule_H
#define WaveTimingFastModule_H

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <topcaf/dataobjects/TOPCAFDigit.h>
#include "TH1D.h"
#include "TH2F.h"

namespace Belle2 {


  /**
   * Module for the hit extraction from pedestal-subtracted waveforms.
   *
   * In order to select both positive and negative pulses at the same time, the waveform is treated in absolute value.
   * Hist are constructed starting from a series of 3 or more consecutive bins with amplitude greated than the threshold, followed by
   * either a bin below threshold or a bin with different amplidue sign.
   * After a segment is found, the hit amplitude is assigned according to the maximum bin inside it. The leading edge crossing time at a given fraction
   * of the maximum amplidute is assigned as hit time. The crossing is calculated with a linear interpolation using the first bin below and bove the theshold.
   * Using the same procedure the crossing time on the second pulse edge is found, and the differnce between the two times is assigned as hit width.
   *
   * The user can specify 3 different thresholds:
   * - m_segmentThreshold: Threshold for the definition of the segment. A single value is used, therefore the algorith will search for both sequences
   *   of positive amplitudes above m_segmentThreshold and sequences of negative amplitudes below -m_globalThreshold.
   * - m_positiveHitThreshold: Threshold for saving a postive hit candidate. Only hits with amplitude greater then m_positiveHitThreshold will be stored
   * - m_negativeHitThreshold: Threshold for saving a negative hit candidate. Only hits with amplitude lower than m_negativeHitThreshold will be stored
   *
   * Known problems:
   * - If the value of m_segmentThreshold is too low, many spurious hits will arise, or two consecutive pulses may be merged in one single segment.
   * - If a pulse produces a segment close to the end of the waveform, so that the amplitude doesn't falls below the threshold before the end of the waveform, the hit is not stored
   * - The hit parameters are calculated assuming thatt the baselina below the hit is flat at zero. If a pulse occurs in a region where the baseline is not zero,  for example because of the afterpulse of a previous signal, the amplitude and width measurment will not take it into account.
   */
  class WaveTimingFastModule : public Module {
  public:

    WaveTimingFastModule();
    ~WaveTimingFastModule();

    //
    void initialize();
    void beginRun();
    void event();
    void terminate();
    //    void defineHisto();

  private:
    double m_fraction, m_time_delay, m_rate, m_thresh, m_frac;
    double m_crude_time, m_cf_time;
    StoreArray<TOPCAFDigit>  m_topcafdigits_ptr;
    double m_time2tdc;
    bool m_dTcalib; /*< Switch to apply the time base corrections */

    /**
     * Calculates the crossing time of a give fraction of the maximum amplitude on the rising edge
     * - v_samples is the waveform as vector of double
     * - tmax is the time bin in which the maximum has been found
     * - amp is the amplitude of the maximum
     * - frac is the relative threshold
     */
    double calcCrossTimeRise(std::vector<double> v_samples, unsigned int tmax, double amp, double frac);

    /**
     * Calculates the crossing time of a give fraction of the maximum amplitude on the falling edge
     * - v_samples is the waveform as vector of double
     * - tmax is the time bin in which the maximum has been found
     * - amp is the amplitude of the maximum
     * - frac is the relative threshold
     */
    double calcCrossTimeFall(std::vector<double> v_samples, unsigned int tmax, double amp, double frac);

    /**
     * Calculates the integral in a +- 2 sigma range w/ respect to the peak position
     * - v_samples is the waveform as vector of double
     * - tmax is the time bin in which the maximum has been found
     * - width is the peak width. The integral is calculated in a +/- 2* width range
     */
    double calcIntegral(std::vector<double> v_samples, unsigned int tmax, double width);

  };

}
#endif
