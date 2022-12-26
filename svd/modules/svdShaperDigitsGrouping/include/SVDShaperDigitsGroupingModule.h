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
#include <framework/dbobjects/HardwareClockSettings.h>
#include <framework/database/DBObjPtr.h>

#include <svd/dataobjects/SVDShaperDigit.h>

#include <svd/reconstruction/SVDMaxSumAlgorithm.h>

#include <svd/calibration/SVDPulseShapeCalibrations.h>
#include <svd/calibration/SVD3SampleCoGTimeCalibrations.h>
#include <svd/calibration/SVDCoGTimeCalibrations.h>
#include <svd/calibration/SVDNoiseCalibrations.h>
#include <svd/calibration/SVDClustering.h>

#include <string>

#include <TFile.h>
#include <TH1D.h>

namespace Belle2 {
  /**
   * Calculate CoG3 time of each strip and group them.
   *
   */
  class SVDShaperDigitsGroupingModule : public Module {

  public:



    /** Constructor */
    SVDShaperDigitsGroupingModule();



    /** Init the module.
     *
     * prepares all store- and relationArrays.
     */
    virtual void initialize() override;

    /** Initialize the list of existing SVD Sensors */
    virtual void beginRun() override;

    /** eventWise jobs (e.g. storing spacepoints */
    virtual void event() override;

    /**
     * set the trigger bin
     */
    void setTriggerBin(const int triggerBin)
    { m_triggerBin = triggerBin; };

  protected:

    /** Name of the collection to use for the SVDEventInfo */
    std::string m_svdEventInfoName;
    /** Name of the collection to use for the SVDShaperDigits */
    std::string m_storeShaperDigitsName;

    // modification parameters
    std::string m_nameOfInstance =
      ""; /**< allows the user to set an identifier for this module. Usefull if one wants to use several instances of that module */

    /** Collection of SVDShaperDigits */
    StoreArray<SVDShaperDigit> m_storeDigits;

    /** trigger bin */
    int m_triggerBin = std::numeric_limits<int>::quiet_NaN();

    /** Hardware Clocks*/
    DBObjPtr<HardwareClockSettings> m_hwClock;
    /** APV clock period*/
    double m_apvClockPeriod = std::numeric_limits<double>::quiet_NaN();

    /**SVDPulseShaper calibration wrapper*/
    SVDPulseShapeCalibrations m_PulseShapeCal;
    /** CoG6 time calibration wrapper*/
    SVDCoGTimeCalibrations m_CoG6TimeCal;
    /** CoG3 time calibration wrapper*/
    SVD3SampleCoGTimeCalibrations m_CoG3TimeCal;

    /**< SVDNoise calibrations db object*/
    SVDNoiseCalibrations m_NoiseCal;
    /**<SVDCluster calibrations db object*/
    SVDClustering m_ClusterCal;

    /** Adjacent cut in units of noise. DEPRECATED - useDB*/
    double m_cutAdjacent = 3;
    /** if true takes the clusterizer cuts and reconstruction configuration from the DB objects*/
    bool m_useDB = true;
  };

} // end namespace Belle2
