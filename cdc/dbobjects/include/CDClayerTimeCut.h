/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TObject.h>
#include "tracking/trackFindingCDC/topology/ISuperLayer.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace Belle2 {

  /**
   * Database object for timing offset (t0).
   */
  class CDClayerTimeCut: public TObject {
  public:

    /**
     * Default constructor
     */
    CDClayerTimeCut() = default;

    /**
     * Constructor using a boost::property_tree
     */
    explicit CDClayerTimeCut(boost::property_tree::ptree tree)
    {
      try {
        int iLayer = 0;
        for (auto& value : tree.get_child("LayerTimeCut")) {

          m_maxDriftTimes[iLayer] = value.second.get_value<float>();
          iLayer += 1;
          if (iLayer > TrackFindingCDC::ISuperLayerUtil::c_N) {
            B2FATAL("Cannot parse CDClayerTimeCut: too many layers");
          }
        }
      } catch (boost::property_tree::ptree_error& e) {
        // Any problem when parsing the tree?
        B2FATAL("Cannot parse CDClayerTimeCut requirement information: " << e.what());
      }


    }

    /**
     * Return cut value for a given layer
     */
    float getLayerTimeCut(int layer) const
    {
      return m_maxDriftTimes[layer];
    }

    /**
     * Dump information
     */
    void dump() const
    {
      std::cout << "Content of CDClayerTimeCut" << std::endl;
      for (int iLayer = 0; iLayer < TrackFindingCDC::ISuperLayerUtil::c_N; iLayer++) {
        std::cout << " Layer :" << iLayer << " Cut:" <<  getLayerTimeCut(iLayer) << std::endl;
      }
    }

  private:
    /// Cut for approximate drift time (super-layer dependent)
    std::array<float, TrackFindingCDC::ISuperLayerUtil::c_N> m_maxDriftTimes = { -1, -1, -1, -1, -1, -1, -1, -1, -1};
    ClassDef(CDClayerTimeCut, 1); /**< ClassDef */
  };

} // end namespace Belle2

