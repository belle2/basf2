/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <map>
#include <string>
#include <TObject.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <iostream>

namespace Belle2 {

  /**
   * Geometry parameters of Beamabort
   */
  class BeamabortGeo:  public TObject {

  public:

    /**
     * Default constructor
     */
    BeamabortGeo()
    {}

    /**
     * Get parameter value
     * @param name parameter name
     */
    double getParameter(const std::string& name) const
    {
      std::map<std::string, double>::const_iterator it = m_params.find(name);
      if (it != m_params.end()) return (*it).second;
      B2FATAL("Requested parameter from BEAMABORT database not found: " << name);
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
     * Get parameter value array
     * @param name parameter name
     */
    std::vector<double> getParArray(const std::string& name) const
    {
      std::map<std::string, std::vector<double>>::const_iterator it = m_par_arrays.find(name);
      if (it != m_par_arrays.end()) return (*it).second;
      B2FATAL("Requested parameter from BEAMABORT database not found: " << name);
    }

    /**
     * Get parameter value array with giving default array in case parameter doesn't exist in DB
     * @param name parameter name
     * @param def default parameter value
     */
    std::vector<double> getParArray(const std::string& name, std::vector<double> def) const
    {
      std::map<std::string, std::vector<double>>::const_iterator it = m_par_arrays.find(name);
      if (it != m_par_arrays.end()) return (*it).second;
      return def;
    }

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
     * Add parameter array to map of parameters
     * @param name parameter name
     * @param arr parameter array
     */
    void addArray(const std::string& name, std::vector<double> arr)
    {
      if (m_par_arrays.insert(std::pair<std::string, std::vector<double>>(name, arr)).second) return;
      else {
        m_par_arrays.find(name)->second = arr;
      }
    }

    /**
     * Get map of all parameters
     */
    const std::map<std::string, double>&  getParameters() const { return m_params;}

    /**
     * Get map of all parameter arrays
     */
    const std::map<std::string, std::vector<double>>&  getParArrays() const { return m_par_arrays;}

    /**
     * Print all parameters
     */
    void print() const
    {
      for (std::pair<std::string, double> element : m_params) {
        std::cout << element.first << " " << element.second << std::endl;
      }
    }

    /**
     * Initialze from the gearbox (xml file)
     * @param content gearbox folder
     */
    void initialize(const GearDir& content);

  private:

    ClassDef(BeamabortGeo, 1); /**< ClassDef */

  protected:

    std::map<std::string, double>  m_params;   /**< map of numeric parameters */
    std::map<std::string, std::vector<double>>  m_par_arrays;   /**< map of numeric parameter arrays */

  };

} // end namespace Belle2
