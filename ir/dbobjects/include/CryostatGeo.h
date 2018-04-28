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

namespace Belle2 {

  /**
   * Geometry parameters of  public TObject {
  public
   */
  class CryostatGeo:  public TObject {

  public:

    /**
     * Default constructor
     */
    CryostatGeo()
    {}

    const std::map<std::string, float>& getParameters() const {return m_params;}

    bool getParameter(std::string& name, float* val)
    {
      std::map<std::string, float>::iterator it = m_params.find(name);
      if (it != m_params.end()) { *val = (*it).second; return true;}
      else return false;
    }

    void addParameter(std::string& name, float val)
    {
      if (m_params.insert(pair<string, float>(name, val)).second) return;
      else {
        //  B2INFO("Parameter value was overwritten: " << name << " " <<  m_params.find(name)->second << " -> " << val);
        m_params.find(name)->second = val;
      }
    }

  private:

    std::map<std::string, float>  m_params;

    ClassDef(CryostatGeo, 1); /**< ClassDef */

  };

} // end namespace Belle2
