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
#include <vxd/dataobjects/VxdID.h>
#include <map>

#include <framework/logging/Logger.h>

namespace Belle2 {

  typedef std::vector<std::pair<double, double>> VXDAlignmentParameters;
  /**
   * Calibration constants of a singe ASIC channel: pedestals, gains and time axis
   */
  class VXDAlignment: public TObject {

  public:


    /**
     * Default constructor
     */
    VXDAlignment()
    {

    }

    /**
     * Destructor
     */
    ~VXDAlignment()
    {
      m_alignment.clear();
    }

    double get(VxdID id, unsigned int param)
    {
      auto sensor = m_alignment.find((unsigned short) id);
      if (sensor == m_alignment.end()) return 0.;

      return sensor->second.at(param).first;
    };
    double getError(VxdID id, unsigned int param)
    {
      auto sensor = m_alignment.find((unsigned short) id);
      if (sensor == m_alignment.end()) return 0.;

      return sensor->second.at(param).second;
    };
    void set(VxdID id, unsigned int param, double value, double error = 0.)
    {
      auto sensor = m_alignment.find((unsigned short) id);
      if (sensor == m_alignment.end()) {
        VXDAlignmentParameters params;
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
        B2INFO(entry.first << " " << (std::string) VxdID(entry.first));
        B2INFO(entry.second.at(0).first);
        B2INFO(entry.second.at(1).first);
        B2INFO(entry.second.at(2).first);
        B2INFO(entry.second.at(3).first);
        B2INFO(entry.second.at(4).first);
        B2INFO(entry.second.at(5).first);
      }
    }

  private:

    std::map<unsigned short, VXDAlignmentParameters> m_alignment; /**< map vxdid -> parameters */

    ClassDef(VXDAlignment, 1); /**< ClassDef */

  };

} // end namespace Belle2

