/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/logging/Logger.h>

#include "TObject.h"
#include "TString.h"
#include <map>

namespace Belle2 {
  /**
   * This class store the shift of the cluster time w.r.t a full calibration.
   * The shift depends on the algorithm, side and layer.
   * The shift should be subtracted from the calibrated
   * cluster time.
   */

  class SVDAbsoluteClusterTimeShift: public TObject {
  public:
    /**
    * Default constructor
    */
    SVDAbsoluteClusterTimeShift(const TString& uniqueID = "")
      : m_uniqueID(uniqueID)
    {
      m_svdAbsTimeShift.clear();
    };

    /**
     * Returns cluster time shift in ns.
     * @param alg : Cluster time algorithm
     * @param layer : layer number
     * @param isU : is U side?
     * @return double : value of the cluster time shift
     **/
    Double_t getAbsTimeShift(const TString& alg,
                             const int& layer, const bool& isU) const
    {
      if (auto searchAlg = m_svdAbsTimeShift.find(alg); // search for time alg
          searchAlg != m_svdAbsTimeShift.end()) {
        TString sensorType = getSensorType(layer, isU);
        if (auto searchShift = (searchAlg->second).find(sensorType); // search for shift values
            searchShift != (searchAlg->second).end())
          return (searchShift->second);
      }
      return 0.;    // returns zero if map is empty
    };

    /**
     * Sets the cluster time shift in ns
     * @param alg : Cluster time algorithm
     * @param sensorType : type of sensor group
     * @param shiftValue : shift value to be set
     */
    void setClusterTimeShift(const TString& alg, const TString& sensorType,
                             const Double_t shiftValue)
    {
      B2INFO("Shift value for " << alg << " and " << sensorType << " is set.");
      m_svdAbsTimeShift[alg][sensorType] = shiftValue;
    };

    /**
     * Get the unique ID  of the payload
     */
    TString get_uniqueID() const {return m_uniqueID;}

    /** name of the payload */
    static std::string name;

  protected:

    /**
     * Returns the types of sensor grouping.
     * @param layer : layer number
     * @param sensor : sensor number
     * @param isU : is U side?
     * @return TString : type of sensor group
     **/
    TString getSensorType(const int& layer, const bool& isU) const
    {
      return TString::Format("L%iS%c", layer, (isU ? 'U' : 'V'));
    };

  private:

    /** unique identifier of the SVD reconstruction configuration payload */
    TString m_uniqueID;

    /** cluster time shifts */
    std::map<TString, std::map<TString, Double_t>> m_svdAbsTimeShift;

    ClassDef(SVDAbsoluteClusterTimeShift, 1); /**< needed by root*/

  };

}
