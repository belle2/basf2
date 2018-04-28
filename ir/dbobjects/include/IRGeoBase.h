/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/gearbox/Unit.h>
#include <map>
#include <string>
#include <TObject.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

namespace Belle2 {

  /**
   * Geometry parameters of  public TObject {
  public
   */
  class IRGeoBase:  public TObject {

  public:

    /**
     * Default constructor
     */

    IRGeoBase()
    {}

    float getParameter(const std::string& name)
    {
      std::map<std::string, float>::iterator it = m_params.find(name);
      if (it != m_params.end()) return (*it).second;
      B2FATAL("Requested parameter from IR database not found: " << name);
      return 0;
    }

    const std::string& getParameterStr(const std::string& name)
    {
      std::map<std::string, std::string>::iterator it = m_strparams.find(name);
      if (it != m_strparams.end()) return (*it).second;
      B2FATAL("Requested parameter from IR database not found: " << name);
      return 0;
    }


    void addParameter(const std::string& name, float val)
    {
      if (m_params.insert(std::pair<std::string, float>(name, val)).second) return;
      else {
        //  B2INFO("Parameter value was overwritten: " << name << " " <<  m_params.find(name)->second << " -> " << val);
        m_params.find(name)->second = val;
      }
    }

    void addParameter(const std::string& name, const std::string& val)
    {
      if (m_strparams.insert(std::pair<std::string, std::string>(name, val)).second) return;
      else {
        //  B2INFO("Parameter value was overwritten: " << name << " " <<  m_params.find(name)->second << " -> " << val);
        m_strparams.find(name)->second = val;
      }
    }

    void addParameters(const GearDir& content, const std::string& section)
    {

      for (const GearDir& slot : content.getNodes(section + "/sec")) {
        std::string name = slot.getString("@name");
        double value;
        if (name.find("A") != std::string::npos) value = slot.getAngle();
        else value = slot.getLength();
        addParameter(section + "." + name, value);
      }
      if (content.exists(section + "/Material")) addParameter(section + ".Material", content.getString(section + "/Material"));
    };

  protected:

    std::map<std::string, float>  m_params;
    std::map<std::string, std::string>  m_strparams;
    ClassDef(IRGeoBase, 1); /**< ClassDef */

  };

} // end namespace Belle2
