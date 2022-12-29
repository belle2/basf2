/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <svd/dataobjects/SVDShaperDigit.h>

#include <svd/calibration/SVDNoiseCalibrations.h>
#include <svd/calibration/SVDClustering.h>

#include <string>
#include <TMath.h>

namespace Belle2 {
  /**
   * Calculate slope of shaper digits.
   *
   */
  class SVDShaperDigitsSlopeModule : public Module {

  public:



    /** Constructor */
    SVDShaperDigitsSlopeModule();



    /** Init the module.
     *
     * prepares all store- and relationArrays.
     */
    virtual void initialize() override;

    /** eventWise jobs (e.g. storing spacepoints */
    virtual void event() override;

    /** Adjacent cut in units of noise. DEPRECATED - useDB*/
    double m_cutAdjacent = 3;
    /** if true takes the clusterizer cuts and reconstruction configuration from the DB objects*/
    bool m_useDB = true;

    /**
     * calculate the slope of shaper digits
     */
    void calculateSlope(SVDShaperDigit& currentDigit);

  protected:

    /** Name of the collection to use for the SVDEventInfo */
    std::string m_svdEventInfoName;
    /** Name of the collection to use for the SVDShaperDigits */
    std::string m_storeShaperDigitsName;

    /** Collection of SVDShaperDigits */
    StoreArray<SVDShaperDigit> m_storeDigits;

    /**< SVDNoise calibrations db object*/
    SVDNoiseCalibrations m_NoiseCal;
    /**<SVDCluster calibrations db object*/
    SVDClustering m_ClusterCal;

  };

} // end namespace Belle2
