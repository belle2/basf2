/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
    explicit MonitoringObject(const std::string& name)
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
     * Get map of all float variables
     */
    const std::map<std::string, float>& getVariables()
    {
      return m_vars;
    }

    /**
     * Get map of all float variables upper errors
     */
    const std::map<std::string, float>& getUpError()
    {
      return m_upErr;
    }

    /**
     * Get map of all float variables lower errors
     */
    const std::map<std::string, float>& getLowError()
    {
      return m_dwErr;
    }

    /**
     * Get vector of all string variables
     */
    const std::vector<std::pair<std::string, std::string>>& getStringVariables()
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

    /**
     * Get list of all canvases
     */
    const std::vector<TCanvas*>& getListOfCanvases()
    {
      return m_Canvases;
    }

    /**
     * Print content of MonitoringObject
     */
    void print() const;

    /**
     * set value to float variable (new variable is made if not yet existing)
     * @param var variable name
     * @param val variable value
     * @param upErr variable size of positive error
     * @param dwErr variable size of negative error (if not set, symmetric error with size upErr is used)
     */
    void setVariable(const std::string& var, float val, float upErr = -1., float dwErr = -1)
    {

      auto vv = m_vars.find(var);
      if (vv != m_vars.end()) vv->second = val;
      else m_vars.insert({var, val});

      if (upErr > 0) {
        auto vvE = m_upErr.find(var);
        if (vvE != m_upErr.end()) vvE->second = upErr;
        else m_upErr.insert({var, upErr});
      }

      if (dwErr > 0) {
        auto vvE = m_dwErr.find(var);
        if (vvE != m_dwErr.end()) vvE->second = dwErr;
        else m_dwErr.insert({var, dwErr});
      }

    }

    /**
     * set value to string variable (new variable is made if not yet existing)
     * @param var variable name
     * @param val variable value
     */
    void setVariable(const std::string& var, const std::string& val)
    {

      for (auto& pair : m_strVars) {
        if (pair.first == var) {
          pair.second = val;
          return;
        }
      }
      m_strVars.push_back(std::pair<std::string, std::string>(var, val));
    }


  private:

    std::vector<TCanvas*> m_Canvases; /**< vector of all TCanvases */

    std::map<std::string, float> m_vars;  /**< map of all float variables */
    std::map<std::string, float> m_upErr; /**< map of upper errors of variables */
    std::map<std::string, float> m_dwErr; /**< map of lower errors of variables */

    std::vector<std::pair<std::string, std::string>> m_strVars; /**< map of all string variables */

    ClassDefOverride(MonitoringObject, 1); /**< classdef */

  }; //class

} // namespace Belle2
