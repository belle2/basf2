/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include "TObject.h"
#include "TString.h"
#include <map>
#include <iostream>
#include <vxd/dataobjects/VxdID.h>

namespace Belle2 {
  /**
   * This class store the reconstruction configuration of SVD
   */

  class SVDClusterTimeShifter: public TObject {
  public:
    /**
    * Default constructor
    */
    SVDClusterTimeShifter(const TString& uniqueID = "",
                          const std::vector<TString>& description = {})
      : m_uniqueID(uniqueID)
      , m_description(description)
    {
      m_svdClusterTimeShift.clear();
    };

    /**
     * Returns cluster time shift in ns
     **/
    Double_t getClusterTimeShift(const int& layer, const int& sensor,
                                 const bool& isU, const int& size) const
    {
      TString sensorType = getSensorType(layer, sensor, isU);
      if (auto search = m_svdClusterTimeShift.find(sensorType);
          search != m_svdClusterTimeShift.end()) {
        int maxClusters = (search->second).size();
        if (maxClusters == 0)
          return 0.;
        else if (size > maxClusters)
          return (search->second)[maxClusters - 1];
        else
          return (search->second)[size - 1];
      } else
        return 0.;
    };

    /**
     * Returns cluster time shift in ns.
     * (Alternate function)
     **/
    Double_t getClusterTimeShift(const VxdID& sensorId,
                                 const bool& isU, const int& size) const
    {
      return getClusterTimeShift(sensorId.getLayerNumber(), sensorId.getSensorNumber(), isU, size);
    };

    /**
     * Sets the cluster time shift in ns
     */
    void setClusterTimeShift(const int& layer, const int& sensor, const bool& isU,
                             const std::vector<Double_t>& shiftValues)
    {
      TString sensorType = getSensorType(layer, sensor, isU);
      m_svdClusterTimeShift[sensorType] = shiftValues;
    };

    /**
     * Sets the cluster time shift in ns
     */
    void setClusterTimeShift(const TString& sensorType,
                             const std::vector<Double_t>& shiftValues)
    {
      std::cout << "Shift values for " << sensorType << " is set." << std::endl;
      m_svdClusterTimeShift[sensorType] = shiftValues;
    };

    /**
     * Get the unique ID  of the calibration
     */
    TString get_uniqueID() const {return m_uniqueID;}

    /** name of the payload */
    static std::string name;

  protected:

    /**
     * Returns the types of sensor grouping
     **/
    TString getSensorType(const int& layer, const int& sensor, const bool& isU) const
    {
      TString sensorType = TString::Format("L%i", layer);

      if (layer >= 4) {
        if (sensor == 1)
          sensorType += "-Forward";
        else
          sensorType += "-Barrel";
      }

      if (isU)
        sensorType += "-U";
      else
        sensorType += "-V";

      return sensorType;
    };

  private:

    /** unique identifier of the SVD reconstruction configuration payload */
    TString m_uniqueID;
    /** short descrition of the payload */
    std::vector<TString> m_description;

    /** cluster time shifts */
    std::map<TString, std::vector<Double_t>> m_svdClusterTimeShift;

    ClassDef(SVDClusterTimeShifter, 1); /**< needed by root*/

  };

}
