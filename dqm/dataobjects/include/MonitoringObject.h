/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2015 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr, Martin Ritter                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <TCanvas.h>

#include <vector>
#include <string>
#include <map>

namespace Belle2 {

  /** Metadata information about a file
   *
   *  See BELLE2-NOTE-TE-2015-028: Event, File, and Dataset Metadata for a
   *  detailed definition. Available at: https://docs.belle2.org/record/287?ln=en
   */
  class MonitoringObject : public TObject {

  public:
    /** Constructor.
     */
    //    MonitoringObject();


    const std::string& getName() const {return m_name;}

    //    void setCreationData(const std::string& date, const std::string& release)
    //{m_date = date; m_release = release;}

    void setName(const std::string& name)
    {
      m_name = name;
    }

    void addCanvas(TCanvas* canv)
    {
      m_Canvases.push_back(canv);
    };

    void addVariable(const std::string& var, float val)
    {
      m_vars.insert({var, val});
    }


  private:

    std::string m_name;

    std::vector<TCanvas*> m_Canvases;

    std::map<std::string, float> m_vars;

    std::map<std::string, std::string> m_strVars;

    std::string m_description;

    ClassDefOverride(MonitoringObject, 1); /**< Metadata information about a file. */

  }; //class

} // namespace Belle2
