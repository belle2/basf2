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
#include <iostream>

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

    double getParameter(const std::string& name)
    {
      std::map<std::string, double>::iterator it = m_params.find(name);
      if (it != m_params.end()) return (*it).second;
      B2FATAL("Requested parameter from IR database not found: " << name);
    }

    double getParameter(const std::string& name, double def)
    {
      std::map<std::string, double>::iterator it = m_params.find(name);
      if (it != m_params.end()) return (*it).second;
      return def;
    }

    const std::string& getParameterStr(const std::string& name)
    {
      std::map<std::string, std::string>::iterator it = m_strparams.find(name);
      if (it != m_strparams.end()) return (*it).second;
      B2FATAL("Requested parameter from IR database not found: " << name);
    }

    const std::string& getParameterStr(const std::string& name, const std::string& def)
    {
      std::map<std::string, std::string>::iterator it = m_strparams.find(name);
      if (it != m_strparams.end()) return (*it).second;
      return def;
    }

    const std::map<std::string, double>&  getParameters() { return m_params;}
    const std::map<std::string, std::string>&  getParametersStr() { return m_strparams;}

    void addParameter(const std::string& name, double val)
    {
      if (m_params.insert(std::pair<std::string, double>(name, val)).second) return;
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

      for (const GearDir& slot : content.getNodes("sec")) {
        std::string name = slot.getString("@name");
        std::string unt = slot.getString("@unit");
        double value;
        if (unt.find("rad") != std::string::npos) value = slot.getAngle();
        else value = slot.getLength();
        addParameter(section + "." + name, value);
      }
      if (content.exists("Material")) addParameter(section + ".Material", content.getString("Material"));
      if (content.exists("Intersect")) addParameter(section + ".Intersect", content.getString("Intersect"));
      if (content.exists("MotherVolume")) addParameter(section + ".MotherVolume", content.getString("MotherVolume"));
      if (content.exists("N")) addParameter(section + ".N", double(content.getInt("N")));
      if (content.exists("@type")) addParameter(section + ".type", content.getString("@type"));
    };

    void print()
    {
      for (std::pair<std::string, double> element : m_params) {
        std::cout << element.first << " " << element.second << std::endl;
      }

      for (std::pair<std::string, std::string> element : m_strparams) {
        std::cout << element.first << " " << element.second << std::endl;
      }
    }


  protected:

    std::map<std::string, double>  m_params;
    std::map<std::string, std::string>  m_strparams;
    ClassDef(IRGeoBase, 1); /**< ClassDef */

  };

} // end namespace Belle2
