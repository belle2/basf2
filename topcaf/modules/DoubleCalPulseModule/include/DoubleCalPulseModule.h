/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Umberto Tamponi (tamponi@to.infn.it)                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/



#ifndef DoubleCalPulseModule_H
#define DoubleCalPulseModule_H

#include <map>
#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <topcaf/dataobjects/TOPCAFDigit.h>
#include "TH1D.h"
#include "TH2F.h"

namespace Belle2 {

  /**
   * Module for the time aligniment of the photon hits  with the calibration pulse. The calibration pulse candidates are searched
   * among the hits, either positive or negative, within specific width and amplitude boundaries, on a particular ASIC channel
   * It superseds all the previous version of DobleCalPulse*Module, but stil uses the old version
   * of TOPCAFDigits.
   *
   * The hit quality flag is assigned as a 4 decimal int in the form abcd,  with
   * b: calpulse/photon discriminant: b = 1 calpulse candidate, b = 0 photon candidate
   * cd: if b=1, this is the sequential number of the calpulses, as they appear on the trace: 01 is the first calpulse (used for calibration), 02 is the second one, etc...
   *     if b = 0, then cd = 00
   * a: time alignment flag
   *    a = -1: no calpulsehas beend found. The this has not been aligned
   *    a = 1: only one clapulse has been found and used for the alignment
   *    a = 2: two calpulse candidates have been found. The first one has been used for the alignement
   *    a = 2: more than two calpulse candidates have been found. The first one has been used for the alignement
   *
   * In a good event with two calibration pulse being identified, the event should have:
   * - one hit with flag = 2101 (first calpulse, alignment done)
   * - one hit with flag = 2102 (second calpulse, alignment done)
   * - all the other hits with flag = 2000 (two calpulse found, alignment done)
   *
   * If only one calpulse has been found, then the event should have:
   * - one hit with flag = 1101 (first calpulse, alignment done)
   * - all the other hits with flag = 1000 (one calpulse found, alignment done)
   */
  class DoubleCalPulseModule : public Module {
  public:

    /**
     * Default constructor
     */
    DoubleCalPulseModule();

    /**
     * Default destructor
     */
    ~DoubleCalPulseModule();

    /**
     * Initializes the class
     */
    void initialize();

    /**
     * Starts the analysis
     */
    void beginRun();

    /**
     * Main loop
     */
    void event();

    /**
     * Ends the analysis
     */
    void terminate();

  private:
    int m_calCh; /**< Channel with calibration pulse */
    double m_wMin; /**< Minimum width for a calibration pulse candidate, in time bins */
    double m_wMax; /**< Maximum width for a calibration pulse candidate, in time bins */
    double m_adcMin; /**< Minumum amplitude for a positive calibration pulse candidate, in adc counts */
    double m_adcMax; /**< Maximum amplitude for a positive calibration pulse candidate, in adc counts */
    double m_adcMinNeg; /**< Minumum amplitude for a negative calibration pulse candidate, in adc counts */
    double m_adcMaxNeg; /**< Minumum amplitude for a negative calibration pulse candidate, in adc counts */

  };

}
#endif
