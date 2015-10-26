/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <cdc/dataobjects/WireID.h>
#include <map>

#include <framework/logging/Logger.h>

namespace Belle2 {

  typedef std::vector<std::pair<double, double>> CDCCalibrationParameters;

  /// CDC alignment and calibration constants
  class CDCCalibration: public TObject {

  public:

    /// Constructor
    CDCCalibration() {}

    /// Destructor
    ~CDCCalibration() {m_alignment.clear();}

    /// Get parameter value for given WireID and parameter number
    double get(WireID id, unsigned int param)
    {
      auto sensor = m_alignment.find((unsigned short) id);
      if (sensor == m_alignment.end()) return 0.;

      return sensor->second.at(param).first;
    };

    /// Get parameter error for given WireID and parameter number
    double getError(WireID id, unsigned int param)
    {
      auto sensor = m_alignment.find((unsigned short) id);
      if (sensor == m_alignment.end()) return 0.;

      return sensor->second.at(param).second;
    };

    /// Set parameter correction (and error) for given WireID and parameter number
    void set(WireID id, unsigned int param, double value, double error = 0.)
    {
      auto sensor = m_alignment.find((unsigned short) id);
      if (sensor == m_alignment.end()) {
        CDCCalibrationParameters params;
        params.push_back({0., 0.});
        params.push_back({0., 0.});
        params.push_back({0., 0.});
        params.push_back({0., 0.});
        params.push_back({0., 0.});
        params.push_back({0., 0.});
        params.at(param).first = value;
        params.at(param).second = error;

        m_alignment.insert(std::make_pair((unsigned short) id, params));

        return;
      };

      sensor->second.at(param).first = value;
      sensor->second.at(param).second = error;
    };

    void dump()
    {
      for (auto& entry : m_alignment) {
        B2INFO(entry.first << " " << WireID(entry.first));
        B2INFO(entry.second.at(0).first);
        B2INFO(entry.second.at(1).first);
        B2INFO(entry.second.at(2).first);
        B2INFO(entry.second.at(3).first);
        B2INFO(entry.second.at(4).first);
        B2INFO(entry.second.at(5).first);
      }
    }

  private:

    std::map<unsigned short, CDCCalibrationParameters> m_alignment; /**< map wireid -> parameters */

    ClassDef(CDCCalibration, 1); /**< ClassDef */

  };

} // end namespace Belle2

