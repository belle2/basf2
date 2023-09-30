/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysis.h
// Description : Baseclass for histogram analysis module for DQM
//-

#pragma once

#include <framework/core/Module.h>
#include <dqm/core/MonitoringObject.h>
#include <dqm/analysis/HistObject.h>
#include <dqm/analysis/HistDelta.h>
#include <TFile.h>
#include <TH1.h>

#include <string>
#include <map>

#ifdef _BELLE2_EPICS
// EPICS
#include "cadef.h"
#endif

namespace Belle2 {

  /**
   * The base class for the histogram analysis module.
   */
  class DQMHistAnalysisModule : public Module {

  public:
    /**
     * The type of list of histograms.
     */
    typedef std::map<std::string, HistObject> HistList;
    /**
     * The type of list of MonitoringObjects.
     */
    typedef std::map<std::string, MonitoringObject*> MonObjList;

    /**
     * The type of list of delta settings and histograms.
     */
    typedef std::map<std::string, HistDelta*> DeltaList;

    /**
     * The type of list of canvas updated status.
     */
    typedef std::map<std::string, bool> CanvasUpdatedList;

  private:
    /**
     * The list of Histograms.
     */
    static HistList s_histList;
    /**
     * The list of MonitoringObjects.
     */
    static MonObjList s_monObjList;

    /**
     * The list of Delta Histograms and settings.
     */
    static DeltaList s_deltaList;

    /**
     * The list of canvas updated status.
     */
    static CanvasUpdatedList s_canvasUpdatedList;

    /**
     * Number of Events processed to fill histograms.
     * Attention: histograms are updates asynchronously
     * Thus the number for a specific histogram may be lower or
     * higher. If you need precise number, you must fill
     * it in the histogram itself (e.g. underflow bin)
     */
    inline static int s_eventProcessed = 0;

    /**
     * The Run type.
     */
    inline static std::string s_runType = "";

    /**
     * Flag if to use EPICS
     * do not set by yourself, use EpicsEnable module to set.
     */
    static bool m_useEpics;

    /**
     * Flag if to use EPICS in ReadOnly mode (for reading limits)
     * do not set by yourself, use EpicsEnable module to set.
     */
    static bool m_epicsReadOnly;

    /**
     * The Prefix for EPICS PVs
     */
    static std::string m_PVPrefix;


#ifdef _BELLE2_EPICS
    //! Vector of EPICS PVs
    std::vector <chid>  m_epicsChID;
    //! Map of (key)names to EPICS PVs
    std::map <std::string, chid> m_epicsNameToChID;
#endif

  public:
    /**
     * Get the list of the histograms.
     * @return The list of the histograms.
     */
    static /*const*/ HistList& getHistList() { return s_histList;};

    /**
     * Get the list of MonitoringObjects.
     * @return The list of the MonitoringObjects.
     */
    static const MonObjList& getMonObjList() { return s_monObjList;};

    /**
     * Get the list of the delta histograms.
     * @return The list of the delta histograms.
     */
    static const DeltaList& getDeltaList() { return s_deltaList;};

    /**
     * Get the list of the canvas update status.
     * @return The list of the canvases.
     */
    static const CanvasUpdatedList& getCanvasUpdatedList() { return s_canvasUpdatedList;};

    /**
     * Get the Run Type.
     * @return Run type string.
     */
    static const std::string& getRunType(void) { return s_runType;};

    /**
     * Get the number of processed events. (Attention, asynch histogram updates!)
     * @return Processed events.
     */
    static int getEventProcessed(void) { return s_eventProcessed;};

    /**
     * Set the Run Type.
     * @par t Run type string.
     */
    void setRunType(std::string& t) {s_runType = t;};

    /**
     * Set the number of processed events. (Attention, asynch histogram updates!)
     * @par e Processed events.
     */
    void setEventProcessed(int e) {s_eventProcessed = e;};

    /**
     * Find canvas by name
     * @param cname Name of the canvas
     * @return The pointer to the canvas, or nullptr if not found.
     */
    TCanvas* findCanvas(TString cname);

    /**
     * Get histogram from list (no other search).
     * @param histname The name of the histogram (incl dir).
     * @param onlyIfUpdated req only updated hists, return nullptr otherwise
     * @return The found histogram, or nullptr if not found.
     */
    static TH1* findHist(const std::string& histname, bool onlyIfUpdated = false);

    /**
     * Find histogram.
     * @param dirname  The name of the directory.
     * @param histname The name of the histogram.
     * @param onlyIfUpdated req only updated hists, return nullptr otherwise
     * @return The found histogram, or nullptr if not found.
     */
    static TH1* findHist(const std::string& dirname,
                         const std::string& histname, bool onlyIfUpdated = false);

    /**
     * Find histogram in specific TFile (e.g. ref file).
     * @param file  The TFile to search.
     * @param histname The name of the histogram, can incl directory
     * @return The found histogram, or nullptr if not found.
     */
    static TH1* findHistInFile(TFile* file, const std::string& histname);

    /**
     * Find histogram in corresponding canvas.
     * @param hname Name of the histogram (dir+name)
     * @return The pointer to the histogram, or nullptr if not found.
     */
    TH1* findHistInCanvas(const std::string& hname);

    /**
     * Find MonitoringObject.
     * @param objName The name of the MonitoringObject.
     * @return The found MonitoringObject, or nullptr if not found.
     */
    static MonitoringObject* findMonitoringObject(const std::string& objName);

    /**
     * Helper function to compute half of the central interval covering 68% of a distribution.
     * This quantity is an alternative to the standard deviation.
     * @param h histogram
     * @return Half of the central interval covering 68% of a distribution.
     */
    double getSigma68(TH1* h) const;

  public:
    /**
     * Add histogram.
     * @param dirname The name of the directory.
     * @param histname The name of the histogram.
     * @param h The TH1 pointer for the histogram.
     * @return histogram was updated flag
     */
    static bool addHist(const std::string& dirname,
                        const std::string& histname, TH1* h);

    /**
     * Get MonitoringObject with given name (new object is created if non-existing)
     * @param histname name of MonitoringObject to get
     * @return The MonitoringObject
     */
    static MonitoringObject* getMonitoringObject(const std::string& histname);

    /**
     * Reset the list of histograms.
     */
    static void initHistListBeforeEvent(void);

    /**
     * Clears the list of histograms.
     */
    static void clearHistList(void);

    /**
     * Get Delta histogram.
     * @param fullname directory+name of histogram
     * @param n index of delta histogram, 0 is most recent one
     * @param onlyIfUpdated req only updated deltas, return nullptr otherwise
     * @return delta histogram or nullptr
     */
    TH1* getDelta(const std::string& fullname, int n = 0, bool onlyIfUpdated = true);

    /**
     * Get Delta histogram.
     * @param dirname directory
     * @param histname name of histogram
     * @param n index of delta histogram, 0 is most recent one
     * @param onlyIfUpdated req only updated deltas, return nullptr otherwise
     * @return delta histogram or nullptr
     */
    TH1* getDelta(const std::string& dirname, const std::string& histname, int n = 0, bool onlyIfUpdated = true);

    /**
     * Add Delta histogram parameters.
     * @param dirname directory
     * @param histname name of histogram
     * @param t type of delta histogramming
     * @param p numerical parameter depnding on type, e.g. number of entries
     * @param a amount of histograms in the past
     */
    void addDeltaPar(const std::string& dirname, const std::string& histname,  HistDelta::EDeltaType t, int p, unsigned int a = 1);

    /**
     * Check if Delta histogram parameters exist for histogram.
     * @param dirname directory
     * @param histname name of histogram
     * @return true if parameters have been set already
     */
    bool hasDeltaPar(const std::string& dirname, const std::string& histname);

    /**
     * Mark canvas as updated (or not)
     * @param name name of Canvas
     * @param updated was updated
     */
    void UpdateCanvas(std::string name, bool updated = true);

    /**
     * Extract Run Type from histogram title, called from input module
     */
    void ExtractRunType(std::vector <TH1*>& hs);

    /**
     * Extract event processed from daq histogram, called from input module
     */
    void ExtractEvent(std::vector <TH1*>& hs);

    /// EPICS related Functions

    /**
     * Register a PV with its name and a key name
     *
     * If you register large number of PVs at once, consider setting
     * update_pvs = false and explicitly running updateEpicsPVs()
     *
     * @param pvname full PV name
     * @param keyname key name for easier access
     * @param update_pvs if true, update all PVs (flush network) after new PV is registered
     * @return an index which can be used to access the PV instead of key name, -1 if failure
     */
    int registerEpicsPV(std::string pvname, std::string keyname = "", bool update_pvs = true);

    /**
     * Write value to a EPICS PV
     * @param keyname key name (or full PV name) of PV
     * @param value value to write
     */
    void setEpicsPV(std::string keyname, double value);

    /**
     * Write value to a EPICS PV
     * @param keyname key name (or full PV name) of PV
     * @param value value to write
     */
    void setEpicsPV(std::string keyname, int value);

    /**
     * Write value to a EPICS PV
     * @param index index of PV
     * @param value value to write
     */
    void setEpicsPV(int index, double value);

    /**
     * Write value to a EPICS PV
     * @param index index of PV
     * @param value value to write
     */
    void setEpicsPV(int index, int value);

    /**
     * Update all EPICS PV (flush to network)
     * @param timeout maximum time until timeout in s
     * */
    void updateEpicsPVs(float timeout);

    /**
     * Get EPICS PV Channel Id
     * @param keyname key name (or full PV name) of PV
     * @return Channel ID is written on success, otherwise nullptr
     */
    chid getEpicsPVChID(std::string keyname);

    /**
     * Get EPICS PV Channel Id
     * @param index index of PV
     * @return Channel ID is written on success, otherwise nullptr
     */
    chid getEpicsPVChID(int index);

    /**
     * Get Alarm Limits from EPICS PV
     * @param id Channel ID
     * @param &lowerAlarm return low Alarm limit (lolo) if set, not changed otherwise
     * @param &lowerWarn return low Warning limit (low) if set, not changed otherwise
     * @param &upperWarn return upper Warning limit (high) if set, not changed otherwise
     * @param &upperAlarm return upper Alarm limit (hihi) if set, not changed otherwise
     * @return true if limits could be read (even if there are none set)
     */
    bool requestLimitsFromEpicsPVs(chid id, double& lowerAlarm, double& lowerWarn, double& upperWarn, double& upperAlarm);

    /**
     * Get Alarm Limits from EPICS PV
     * @param keyname key name (or full PV name) of PV
     * @param &lowerAlarm return low Alarm limit (lolo) if set, not changed otherwise
     * @param &lowerWarn return low Warning limit (low) if set, not changed otherwise
     * @param &upperWarn return upper Warning limit (high) if set, not changed otherwise
     * @param &upperAlarm return upper Alarm limit (hihi) if set, not changed otherwise
     * @return true if limits could be read (even if there are none set)
     */
    bool requestLimitsFromEpicsPVs(std::string keyname, double& lowerAlarm, double& lowerWarn, double& upperWarn, double& upperAlarm);

    /**
     * Get Alarm Limits from EPICS PV
     * @param index index of PV
     * @param &lowerAlarm return low Alarm limit (lolo) if set, not changed otherwise
     * @param &lowerWarn return low Warning limit (low) if set, not changed otherwise
     * @param &upperWarn return upper Warning limit (high) if set, not changed otherwise
     * @param &upperAlarm return upper Alarm limit (hihi) if set, not changed otherwise
     * @return true if limits could be read (even if there are none set)
     */
    bool requestLimitsFromEpicsPVs(int index, double& lowerAlarm, double& lowerWarn, double& upperWarn, double& upperAlarm);

    /**
     * Setter for EPICS usage
     * @param flag set in use
     */
    void setUseEpics(bool flag) {m_useEpics = flag;};

    /**
     * Setter EPICS flag in read only mode
     * @param flag set read only
     */
    void setUseEpicsReadOnly(bool flag) {m_epicsReadOnly = flag;};

    /**
     * Getter for EPICS usage
     * @return flag is in use
     */
    bool getUseEpics(void) {return m_useEpics;};

    /**
     * Getter EPICS flag in read only mode
     * @return flag if read only
     */
    bool getUseEpicsReadOnly(void) {return m_epicsReadOnly;};

    /**
     * Unsubscribe from EPICS PVs on terminate
     */
    void cleanupEpicsPVs(void);

    /**
     * get global Prefix for EPICS PVs
     * @return prefix in use
     */
    std::string& getPVPrefix(void) {return m_PVPrefix;};

    /**
     * set global Prefix for EPICS PVs
     * @param prefix Prefix to set
     */
    void setPVPrefix(std::string& prefix) { m_PVPrefix = prefix;};

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisModule();
    virtual ~DQMHistAnalysisModule() {};

    /**
     * Helper function for string token split
     * @param s String to split
     * @param delim delimiter
     * @return vector of strings
     */
    std::vector <std::string> StringSplit(const std::string& s, const char delim);

    // Data members
  private:
  };
} // end namespace Belle2

