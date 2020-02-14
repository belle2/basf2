/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2015 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <TNamed.h>
#include <TCanvas.h>

#include <vector>
#include <string>
#include <map>
#include <framework/logging/Logger.h>

namespace Belle2 {

  /**
   *  MonitoringObject is a basic object to hold data for the run-dependency monitoring
   *  Run summary TCanvases and monitoring variables
   */

  class MonitoringObject : public TNamed {

  public:
    /** Constructor.
     */
    MonitoringObject() {};
    /** Constructor with name (always use this)
     */
    MonitoringObject(const std::string& name)
    {
      fName = name;
    }

    /**
     * Add Canvas to monitoring object
     * @param canv pointer to canvas to add
     */
    void addCanvas(TCanvas* canv)
    {
      for (auto cc : m_Canvases) {
        if (cc->GetName() == canv->GetName()) {
          B2ERROR("Canvas with name " << canv->GetName() << " already in the " << fName <<
                  " MonitoringObject! Use different name (or call getCanvas(name) to access pointer to the existing TCanvas).");
          return;
        }
      }
      m_Canvases.push_back(canv);
    };

    /**
     * set value to float variable (new variable is made if not yet existing)
     * @param var variable name
     * @param val variable value
     * @param err variable error
     */
    void setVariable(const std::string& var, float val, float err = -1)
    {
      auto vv = m_vars.find(var);
      if (vv != m_vars.end()) vv->second = std::pair<float, float>(val, err);
      else m_vars.insert({var, std::pair<float, float>(val, err)});
    }

    /**
     * set value to string variable (new variable is made if not yet existing)
     * @param var variable name
     * @param val variable value
     */
    void setVariable(const std::string& var, const std::string& val)
    {
      auto vv = m_strVars.find(var);
      if (vv != m_strVars.end()) vv->second = val;
      else m_strVars.insert({var, val});
    }

    /**
     * Get map of all float variables
     */
    const std::map<std::string, std::pair<float, float>>& getVariables()
    {
      return m_vars;
    }

    /**
     * Get map of all string variables
     */
    const std::map<std::string, std::string>& getStringVariables()
    {
      return m_strVars;
    }

    /**
     * Get pointer to existing canvas with given name (NULL is returned if not existing)
     */
    TCanvas* getCanvas(const std::string& name)
    {
      for (auto cc : m_Canvases) {
        if (cc->GetName() == name) return cc;
      }
      B2WARNING("TCanvas with name " << name << " not found in MonitoringObject " << fName);
      return NULL;
    };



  private:

    std::vector<TCanvas*> m_Canvases; /**< vector of all TCanvases */

    std::map<std::string, std::pair<float, float>> m_vars; /**< map of all float variables with their errors */

    std::map<std::string, std::string> m_strVars; /**< map of all string variables */

    ClassDefOverride(MonitoringObject, 1); /**< classdef */

  }; //class

} // namespace Belle2
