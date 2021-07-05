/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>
#include <framework/datastore/StoreArray.h>
#include <top/dataobjects/TOPDigit.h>

class TH1F;

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
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     */
    virtual void beginRun() override;

    /**
     * Event processor.
     */
    virtual void event() override;

    /**
     * End-of-run action.
     */
    virtual void endRun() override;

    /**
     * Termination action.
     * Output MC extraction
     */
    virtual void terminate() override;

  private:

    /**
     * number of channels per module, storage windows per channel
     */
    enum {c_NumChannels = 512,
          c_maxLaserFibers = 9
         };

    TH1F* m_histo[c_NumChannels] = {0}; /**< profile histograms */

    std::vector<double> m_fitRange; /**< fit range [nbins, xmin, xmax] */
    std::string m_outputFile; /**< output root file name */

    StoreArray<TOPDigit> m_digits; /**< collection of digits */


  };

} // Belle2 namespace

