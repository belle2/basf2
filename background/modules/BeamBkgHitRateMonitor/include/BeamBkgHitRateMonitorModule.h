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
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/dataobjects/FileMetaData.h>
#include <mdst/dataobjects/TRGSummary.h>
#include <background/modules/BeamBkgHitRateMonitor/HitRateBase.h>

#include <string>
#include <vector>
#include <map>
#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>

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
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate() override;

  private:

    /**
     * event selection
     * @return true if event is selected
     */
    bool isEventSelected();

    /**
     * Collect file meta data: LFN's, low and high experiment, run and event numbers.
     * This code is based on RootOutputModule::event().
     */
    void collectFileMetaData();

    /**
     * Set output FileMetaData object.
     * This code is based on RootOutputModule::fillFileMetaData().
     */
    void setFileMetaData();

    // steering parameters
    std::string m_outputFileName; /**< output file name */
    std::string m_treeName;       /**< output tree name */
    std::vector<int> m_trgTypes;  /**< trigger types to be selected */
    bool m_writeEmptyTimeStamps;  /**< if true write to ntuple also empty time stamps */
    double m_topTimeOffset; /**< TOP: time offset of hits [ns] */
    double m_topTimeWindow; /**< TOP: time window in which to count hits [ns] */
    std::string m_svdShaperDigitsName; /**< SVD: name of the SVDShaperDigits collection */
    double m_svdThrCharge; /**< SVD: energy cut on cluster charge */
    bool m_svdIgnoreHotStrips; /**< SVD: count hot strips as active */
    bool m_svdIgnoreMaskedStrips; /**< SVD: count FADC-masked strips as active */
    std::map<std::string, std::string> m_additionalDataDescription; /**< additional metadata description */
    int m_cdcTimeWindowLowerEdgeSmallCell;  /**< CDC: lower edge of the time window for small cells [tdc count = ns] */
    int m_cdcTimeWindowUpperEdgeSmallCell;  /**< CDC: upper edge of the time window for small cells [tdc count = ns] */
    int m_cdcTimeWindowLowerEdgeNormalCell; /**< CDC: lower edge of the time window for normal cells [tdc count = ns] */
    int m_cdcTimeWindowUpperEdgeNormalCell; /**< CDC: upper edge of the time window for normal cells [tdc count = ns] */
    bool m_cdcEnableBadWireTreatment;       /**< CDC: flag to enable the bad wire treatment */
    bool m_cdcEnableBackgroundHitFilter;    /**< CDC: flag to enable the CDC background hit (crosstakl, noise) filter */
    bool m_cdcEnableMarkBackgroundHit;      /**< CDC: flag to enable to mark background flag on CDCHit (set 0x100 bit for CDCHit::m_status). default: false */

    // collections
    StoreObjPtr<EventMetaData> m_eventMetaData; /**< event meta data object */
    StoreObjPtr<TRGSummary> m_trgSummary;  /**< trigger summary */
    StoreObjPtr<FileMetaData> m_fileMetaData{"", DataStore::c_Persistent}; /**< file metadata */

    // output file and tree
    TFile* m_file = 0; /**< root file pointer */
    TTree* m_tree = 0; /**< root tree pointer */

    // tree variables
    int m_run = 0;  /**< run number */
    int m_numEvents = 0;  /**< number of events in the time stamp */
    unsigned m_timeStamp = 0;  /**< time stamp (unix time) */
    int m_time = 0;  /**< time in seconds w.r.t the first event of the run */

    // control histograms
    TH1F* m_trgAll = 0; /**< trigger types of all events */
    TH1F* m_trgSel = 0; /**< trigger types of selected events */

    // buffers
    std::map<unsigned, int> m_eventCounts; /**< number of events in time stamps */

    // rate monitoring classes: these provide additional buffers and tree branches
    std::vector<Background::HitRateBase*> m_monitors; /**< rate monitors */

    // other
    unsigned m_numEventsSelected = 0;  /**< number of selected events in a run */
    std::map<TRGSummary::ETimingType, unsigned> m_trgTypesCount; /**< trigger type counters */
    unsigned m_utimeFirst = 0; /**< unix time of the first event in the run input stream */
    unsigned m_utimeMin = 0; /**< minimal unix time of events in the run */
    unsigned m_utimeMax = 0; /**< maximal unix time of events in the run */

    // output file meta data
    TTree* m_persistent = 0; /**< root tree pointer (for FileMetaData) */
    FileMetaData m_outputFileMetaData; /**< output file meta data branch */
    std::vector<std::string> m_parentLfns; /**< Vector of parent file LFNs. */
    unsigned long m_experimentLow = 1;  /**< Lowest experiment number. */
    unsigned long m_runLow = 0;         /**< Lowest run number. */
    unsigned long m_eventLow = 0;       /**< Lowest event number in lowest run. */
    unsigned long m_experimentHigh = 0; /**< Highest experiment number.*/
    unsigned long m_runHigh = 0;        /**< Highest run number. */
    unsigned long m_eventHigh = 0;      /**< Highest event number in highest run. */
    unsigned m_allEventsSelected = 0;  /**< number of selected events in all runs */

  };

} // Belle2 namespace

