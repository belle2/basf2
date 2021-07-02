/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysis.h
// Description : Histogram analysis module for DQM
//
// Author : Tomoyuki Konno, Tokyo Metropolitan University
// Date : 25 - Dec - 2015 ; first commit
//-

#pragma once

#include <framework/core/Module.h>
#include <dqm/dataobjects/MonitoringObject.h>
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
     * The enumeration types for the module parameters.
     */
    enum EParamType {
      /**
       * The integer type for module parameter.
       */
      c_ParamINT,
      /**
       * The float type for module parameter.
       */
      c_ParamFLOAT,
      /**
       * The string type for module parameter.
       */
      c_ParamTEXT
    };
    /**
     * The type of list of module parameter types.
     */
    typedef std::map<std::string, EParamType> ParamTypeList;
    /**
     * The type of list of integer module parameter.
     */
    typedef std::map<std::string, int> IntValueList;
    /**
     * The type of list of float module parameter.
     */
    typedef std::map<std::string, float> FloatValueList;
    /**
     * The type of list of string module parameter.
     */
    typedef std::map<std::string, std::string> TextList;
    /**
     * The type of list of histograms.
     */
    typedef std::map<std::string, TH1*> HistList;
    /**
     * The type of list of MonitoringObjects.
     */
    typedef std::map<std::string, MonitoringObject*> MonObjList;



  private:
    /**
     * The list of module parameter types.
     */
    static ParamTypeList g_parname;
    /**
     * The list of integer module parameter.
     */
    static IntValueList g_vint;
    /**
     * The list of float module parameter.
     */
    static FloatValueList g_vfloat;
    /**
     * The list of string module parameter.
     */
    static TextList g_text;
    /**
     * The list of histograms.
     */
    static HistList g_hist;
    /**
     * The list of MonitoringObjects.
     */
    static MonObjList g_monObj;


  public:
    /**
     * Get the list of the histograms.
     * @return The list of the histograms.
     */
    static const HistList& getHistList();
    /**
     * Get the list of MonitoringObjects.
     * @return The list of the MonitoringObjects.
     */
    static const MonObjList& getMonObjList();

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
    /**
     * Set the integer value of the parameter.
     * @param parname The name of the parameter.
     * @param vint The value to be set.
     */
    static void setIntValue(const std::string& parname, int vint);

    /**
     * Set the float value of the parameter.
     * @param parname The name of the parameter.
     * @param vfloat The value to be set.
     */
    static void setFloatValue(const std::string& parname, float vfloat);
    /**
     * Set the string value of the parameter.
     * @param parname The name of the parameter.
     * @param text The value to be set.
     */
    static void setText(const std::string& parname, const std::string& text);

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
    /**
     * Get the list of histograms.
     * @return The list of histograms.
     */
    static HistList& getHists() { return g_hist; }
    /**
     * Get the list of the names and types of the parameters.
     * @return The list of the names and types of the parameters.
     */
    static ParamTypeList& getParNames() { return g_parname; }
    /**
     * Get the list of integer parameters.
     * @return The list of integer parameters.
     */
    static IntValueList& getIntValues() { return g_vint; }
    /**
     * Get the list of float parameters.
     * @return The list of float parameters.
     */
    static FloatValueList& getFloatValues() { return g_vfloat; }
    /**
     * Get the list of string parameters.
     * @return The list of string parameters.
     */
    static TextList& getTexts() { return g_text; }

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisModule();
    virtual ~DQMHistAnalysisModule();

    // Data members
  private:

  };
} // end namespace Belle2

