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
   * This class store the shift in svd time w.r.t. cluster time
   * also for any possible time algorithm
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
     * Returns cluster time shift in ns.
     **/
    Double_t getClusterTimeShift(const TString& alg,
                                 const VxdID& sensorId, const bool& isU, const int& size) const
    {
      if (auto searchAlg = m_svdClusterTimeShift.find(alg); // search for time alg
          searchAlg != m_svdClusterTimeShift.end()) {
        TString sensorType = getSensorType(sensorId.getLayerNumber(), sensorId.getSensorNumber(), isU);
        if (auto searchShift = (searchAlg->second).find(sensorType); // search for shift values
            searchShift != (searchAlg->second).end()) {
          int maxClusters = (searchShift->second).size();
          if (maxClusters == 0)
            return 0.;
          else if (size > maxClusters)
            return (searchShift->second)[maxClusters - 1];
          else
            return (searchShift->second)[size - 1];
        }
      }
      return 0.;    // returns zero if map is empty
    };

    /**
     * Sets the cluster time shift in ns
     */
    void setClusterTimeShift(const TString& alg, const TString& sensorType,
                             const std::vector<Double_t>& shiftValues)
    {
      std::cout << "Shift values for " << alg << " and " << sensorType << " is set." << std::endl;
      m_svdClusterTimeShift[alg][sensorType] = shiftValues;
    };

    /**
     * Get the unique ID  of the payload
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
    std::map<TString, std::map<TString, std::vector<Double_t>>> m_svdClusterTimeShift;

    ClassDef(SVDClusterTimeShifter, 1); /**< needed by root*/

  };

}
