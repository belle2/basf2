/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Roberto Stroili, Wenlong Yuan                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>
#include <framework/datastore/StoreArray.h>
#include <top/dataobjects/TOPDigit.h>

class TH1F;
class TF1;
class TTree;

namespace Belle2 {
  /**
   * T0 Laser calibration module
   * (under development)
   */
  class TOPLaserCalibratorModule : public Module {

  public:
    /**
     * Constructor
     */
    TOPLaserCalibratorModule();

    /**
     * Destructor
     */
    virtual ~TOPLaserCalibratorModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun();

    /**
     * Event processor.
     */
    virtual void event();

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     * Do fits , clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

  private:

    /**
     * number of channels per module, storage windows per channel
     */
    enum {c_NumChannels = 512,
          c_maxLaserFibers = 9
         };

    std::string m_dataFitOutput; /**< output root file for data */
    std::string m_mcInput; /**< Input root file from MC */
    std::string m_chT0C; /**< Output of channel T0 constant */

    int m_barID; /**< ID of TOP module to calibrate */
    int m_refCh; /**< reference channel of T0 constant */
    int m_fitChannel; /**< set 0 - 511 to a specific pixelID in the fit; set 512 to fit all pixels in one slot */
    std::string m_fitMethod; /**< gauss: single gaussian; cb: single Crystal Ball; cb2: double Crystal Ball */
    std::vector<double> m_fitRange; /**< fit range [nbins, xmin, xmax] */
    TH1F* m_histo[c_NumChannels] = {0}; /**< profile histograms */

    StoreArray<TOPDigit> m_digits; /**< collection of digits */

  };
} // Belle2 namespace

