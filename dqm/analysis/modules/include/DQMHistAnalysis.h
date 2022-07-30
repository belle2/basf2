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
#include <dqm/dataobjects/MonitoringObject.h>
#include <dqm/analysis/HistDelta.h>
#include <TH1.h>

#include <string>
#include <map>

namespace Belle2 {

  /**
   * The base class for the histogram analysis module.
   */
  class DQMHistAnalysisModule : public Module {

  public:
    /**
     * The type of list of histograms.
     */
    typedef std::map<std::string, TH1*> HistList;
    /**
     * The type of list of MonitoringObjects.
     */
    typedef std::map<std::string, MonitoringObject*> MonObjList;

    /**
     * The type of list of delta settings and histograms.
     */
    typedef std::map<std::string, HistDelta*> DeltaList;


  private:
    /**
     * The list of Histograms.
     */
    static HistList g_hist;
    /**
     * The list of MonitoringObjects.
     */
    static MonObjList g_monObj;

    /**
     * The list of Delta Histograms and settings.
     */
    static DeltaList g_delta;

  public:
    /**
     * Get the list of the histograms.
     * @return The list of the histograms.
     */
    static const HistList& getHistList() { return g_hist;};

    /**
     * Get the list of MonitoringObjects.
     * @return The list of the MonitoringObjects.
     */
    static const MonObjList& getMonObjList() { return g_monObj;};

    /**
     * Get the list of the delta histograms.
     * @return The list of the delta histograms.
     */
    static const DeltaList& getDeltaList() { return g_delta;};

    /**
     * Find canvas by name
     * @param cname Name of the canvas
     * @return The pointer to the canvas, or nullptr if not found.
     */
    TCanvas* findCanvas(TString cname);

    /**
     * Find histogram.
     * @param histname The name of the histogram.
     * @return The found histogram, or nullptr if not found.
     */
    static TH1* findHist(const std::string& histname);
    /**
     * Find histogram.
     * @param dirname  The name of the directory.
     * @param histname The name of the histogram.
     * @return The found histogram, or nullptr if not found.
     */
    static TH1* findHist(const std::string& dirname,
                         const std::string& histname);
    /**
     * Find histogram.
     * @param histdir  The TDirectory of the directory.
     * @param histname The name of the histogram.
     * @return The found histogram, or nullptr if not found.
     */
    static TH1* findHist(const TDirectory* histdir, const TString& histname);
    /**
     * Find MonitoringObject.
     * @param objName The name of the MonitoringObject.
     * @return The found MonitoringObject, or nullptr if not found.
     */
    static MonitoringObject* findMonitoringObject(const std::string& objName);

  public:
    /**
     * Add histogram.
     * @param dirname The name of the directory.
     * @param histname The name of the histogram.
     * @param h The TH1 pointer for the histogram.
     */
    static void addHist(const std::string& dirname,
                        const std::string& histname, TH1* h);

    /**
     * Get MonitoringObject with given name (new object is created if non-existing)
     * @param histname name of MonitoringObject to get
     */
    static MonitoringObject* getMonitoringObject(const std::string& histname);

    /**
     * Clear and reset the list of histograms.
     */
    static void resetHist() { g_hist = std::map<std::string, TH1*>(); }

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisModule();
    virtual ~DQMHistAnalysisModule();

    // Data members
  private:

  };
} // end namespace Belle2

