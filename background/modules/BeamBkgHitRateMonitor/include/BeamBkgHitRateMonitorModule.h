/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <rawdata/dataobjects/RawTRG.h>
#include <mdst/dataobjects/TRGSummary.h>
#include <background/modules/BeamBkgHitRateMonitor/HitRateBase.h>

#include <string>
#include <vector>
#include <map>
#include <TFile.h>
#include <TTree.h>

namespace Belle2 {

  /**
   * A module to monitor detector hit rates of beam background
   * Output is to a flat ntuple
   */
  class BeamBkgHitRateMonitorModule : public Module {

  public:

    /**
     * Constructor
     */
    BeamBkgHitRateMonitorModule();

    /**
     * Destructor
     */
    virtual ~BeamBkgHitRateMonitorModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Event processor.
     */
    virtual void event() override;

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate() override;

  private:

    /**
     * event selection
     * @return true if event is selected
     */
    bool isEventSelected();

    // steering parameters
    std::string m_outputFileName; /**< output file name */
    std::string m_treeName;       /**< output tree name */
    std::vector<int> m_trgTypes;  /**< trigger types to be selected */
    bool m_writeEmptyTimeStamps;  /**< if true write to ntuple also empty time stamps */
    double m_topTimeOffset; /**< TOP: time offset of hits [ns] */
    double m_topTimeWindow; /**< TOP: time window in which to count hits [ns] */

    // collections
    StoreObjPtr<EventMetaData> m_eventMetaData; /**< event meta data object */
    StoreArray<RawTRG> m_rawTRGs;  /**< collection of raw triggers */
    StoreObjPtr<TRGSummary> m_trgSummary;  /**< trigger summary */

    // output file and tree
    TFile* m_file = 0; /**< root file pointer */
    TTree* m_tree = 0; /**< root tree pointer */

    // tree variables
    int m_run = 0;  /**< run number */
    int m_numEvents = 0;  /**< number of events in time stamp */
    unsigned m_timeStamp = 0;  /**< time stamp (unix time) */
    int m_time = 0;  /**< time in seconds w.r.t the first event */

    // buffers
    std::map<unsigned, int> m_runNumbers; /**< run numbers of time stamps */
    std::map<unsigned, int> m_eventCounts; /**< number of events in time stamps */

    // rate monitoring classes: these provide additional buffers and tree branches
    std::vector<Background::HitRateBase*> m_monitors; /**< rate monitors */

    // other
    unsigned m_numEventsSelected = 0;  /**< number of selected events */
    std::vector<unsigned> m_trgTypesCount; /**< trigger type counter */
    unsigned m_utimeFirst = 0; /**< unix time of the first event in the input stream */
    unsigned m_utimeMin = 0; /**< minimal unix time of the events */
    unsigned m_utimeMax = 0; /**< maximal unix time of the events */

  };

} // Belle2 namespace

