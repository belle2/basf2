/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Wenlong Yuan                                             *
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
   * TOP Channel T0 MC Extraction module
   * (under development)
   */
  class TOPChannelT0MCModule : public Module {

  public:
    /**
     * Constructor
     */
    TOPChannelT0MCModule();

    /**
     * Destructor
     */
    virtual ~TOPChannelT0MCModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     */
    virtual void beginRun();

    /**
     * Event processor.
     */
    virtual void event();

    /**
     * End-of-run action.
     */
    virtual void endRun();

    /**
     * Termination action.
     * Output MC extraction
     */
    virtual void terminate();

  private:

    /**
     * number of channels per module, storage windows per channel
     */
    enum {c_NumChannels = 512,
          c_maxLaserFibers = 9
         };

    TH1F* m_histo[c_NumChannels]; /**< profile histograms */

    std::vector<double> m_fitRange; /**< fit range [nbins, xmin, xmax] */
    std::string m_outputFile; /**< output root file name */

    StoreArray<TOPDigit> m_digits; /**< collection of digits */


  };

} // Belle2 namespace

