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
   * Base class for IR geometry parameters
   */
  class IRGeoBase:  public TObject {

  public:

    /**
     * Default constructor
     */
    IRGeoBase()
    {}

    /**
     * Get parameter value
     * @param name parameter name
     */
    double getParameter(const std::string& name) const
    {
      std::map<std::string, double>::const_iterator it = m_params.find(name);
      if (it != m_params.end()) return (*it).second;
      B2FATAL("Requested parameter from IR database not found: " << name);
    }

    /**
     * Get parameter value with giving default value in case parameter doesn't exist in DB
     * @param name parameter name
     * @param def default parameter value
     */
    double getParameter(const std::string& name, double def) const
    {
      std::map<std::string, double>::const_iterator it = m_params.find(name);
      if (it != m_params.end()) return (*it).second;
      return def;
    }

    /**
     * Get string parameter
     * @param name parameter name
     */
    const std::string& getParameterStr(const std::string& name) const
    {
      std::map<std::string, std::string>::const_iterator it = m_strparams.find(name);
      if (it != m_strparams.end()) return (*it).second;
      B2FATAL("Requested parameter from IR database not found: " << name);
    }

    /**
     * Get string parameter with giving default value in case of non-existence
     * @param name parameter name
     */
    const std::string& getParameterStr(const std::string& name, const std::string& def) const
    {
      std::map<std::string, std::string>::const_iterator it = m_strparams.find(name);
      if (it != m_strparams.end()) return (*it).second;
      return def;
    }

    /**
     * Get map of all parameters
     */
    const std::map<std::string, double>&  getParameters() const { return m_params;}

    /**
     * Get map of all string parameters
     */
    const std::map<std::string, std::string>&  getParametersStr() const { return m_strparams;}

    /**
     * Add parameter to map of parameters
     * @param name parameter name
     * @param val parameter value
     */
    void addParameter(const std::string& name, double val)
    {
      if (m_params.insert(std::pair<std::string, double>(name, val)).second) return;
      else {
        m_params.find(name)->second = val;
      }
    }

    /**
     * Add string parameter to map of parameters
     * @param name parameter name
     * @param val parameter value
     */
    void addParameter(const std::string& name, const std::string& val)
    {
      if (m_strparams.insert(std::pair<std::string, std::string>(name, val)).second) return;
      else {
        m_strparams.find(name)->second = val;
      }
    }

    /**
     * Add parameters from Gearbox
     * @param content gearbox folder
     * @param section name of section of parameters to add
     */
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

    /**
     * Print all parameters
     */
    void print() const
    {
      for (std::pair<std::string, double> element : m_params) {
        std::cout << element.first << " " << element.second << std::endl;
      }

      for (std::pair<std::string, std::string> element : m_strparams) {
        std::cout << element.first << " " << element.second << std::endl;
      }
    }


  protected:

    std::map<std::string, double>  m_params;   /**< map of numeric parameters */
    std::map<std::string, std::string>  m_strparams; /**< map of string parameters */
    ClassDef(IRGeoBase, 1); /**< ClassDef */

  };

} // end namespace Belle2
