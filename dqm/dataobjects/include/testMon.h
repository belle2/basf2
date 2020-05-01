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
   *  testMon is a basic object to hold data for the run-dependency monitoring
   *  Run summary TCanvases and monitoring variables
   */

  class testMon : public TNamed {

  public:
    /** Constructor.
     */
    testMon() {};
    /** Constructor with name (always use this)
     */
    testMon(const std::string& name)
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

    void setVariable(const std::string& var, float val, float upErr = -1., float dwErr = -1)
    {

      auto vv = m_vars.find(var);
      if (vv != m_vars.end()) vv->second = val;
      else m_vars.insert({var, val});

      if (upErr > 0) {
        auto vv = m_upErr.find(var);
        if (vv != m_upErr.end()) vv->second = upErr;
        else m_upErr.insert({var, upErr});
      }

      if (dwErr > 0) {
        auto vv = m_dwErr.find(var);
        if (vv != m_dwErr.end()) vv->second = dwErr;
        else m_dwErr.insert({var, dwErr});
      }

    }

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

    std::map<std::string, float> m_vars;
    std::map<std::string, float> m_upErr;
    std::map<std::string, float> m_dwErr;

    std::vector<std::pair<std::string, std::string>> m_strVars;

    ClassDefOverride(testMon, 1); /**< classdef */

  }; //class

} // namespace Belle2


