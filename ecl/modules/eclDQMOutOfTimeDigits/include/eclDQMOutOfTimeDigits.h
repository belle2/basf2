/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* Basf2 headers. */
#include <framework/core/HistoModule.h>
#include <framework/core/ModuleParam.templateDetails.h>
#include <framework/datastore/StoreObjPtr.h>

class TProfile;

namespace Belle2 {

  class TRGSummary;

  /**
   * This module produces DQM histograms to monitor the distribution of out-of-time ECLCalDigits.
   * ("out-of-time ECLCalDigits" -> number of crystals with E > 7 MeV and abs(time) > 110 ns
   *  (time relative to the trigger time)
   * )
   * High value of this observable leads means that ECL is strongly affected by beam background.
   * This likely means degradation in data quality, as was demonstrated in
   * https://indico.belle2.org/event/9378/contributions/61260/attachments/22475/33166/ECL%20high%20backgrounds%2025-May-2023.pdf
   */
  class ECLDQMOutOfTimeDigitsModule : public HistoModule {  /**< derived from HistoModule class. */

  public:

    /** Constructor. */
    ECLDQMOutOfTimeDigitsModule();

    /** Destructor. */
    virtual ~ECLDQMOutOfTimeDigitsModule();

    /** Initialize the module. */
    virtual void initialize() override;
    /** Call when a run begins. */
    virtual void beginRun() override;
    /** Event processor. */
    virtual void event() override;
    /** Call when a run ends. */
    virtual void endRun() override;
    /** Terminate. */
    virtual void terminate() override;

    /** Function to define histograms. */
    virtual void defineHisto() override;

  private:

    /**
     * @return true if the current event is a random trigger event,
     *         false otherwise.
     */
    bool isRandomTrigger();

    /** Return type of the current event */
    std::string getEventType();

    /**
     * Single-value histograms to hold the average
     * value for out-of-time ECLCalDigits.
     *
     * All keys follow this format
     * {event_type}_{ecl_part}
     * event_type: rand (random trigger evts), dphy (delayed bhabha events), physics (other events)
     * ecl_part: all, fwd, bar, bwd
     */
    std::map<std::string, TProfile*> h_out_of_time{};

    /** StoreObjPtr TRGSummary  */
    StoreObjPtr<TRGSummary> m_l1Trigger;

    /** Histogram directory in ROOT file. */
    std::string m_histogramDirectoryName;
  };
}; // end Belle2 namespace
