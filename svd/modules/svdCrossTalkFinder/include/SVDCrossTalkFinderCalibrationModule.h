/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: James Webb                                               *
 *                                                                        *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <vxd/geometry/GeoCache.h>
#include <vxd/dataobjects/VxdID.h>
#include <svd/dataobjects/SVDRecoDigit.h>

#include <string>
#include <map>

#include <TFile.h>
#include <TH1.h>

namespace Belle2 {
  /**
   * Generate sensor wise occupancy files for use in
   * cross-talk cluster finding.
   */
  class SVDCrossTalkFinderCalibrationModule : public Module {

  public:
    /** Constructor */
    SVDCrossTalkFinderCalibrationModule();



    /** Init the module. */
    virtual void initialize() override;


    /** eventWise jobs */
    virtual void event() override;


    /** final output  */
    virtual void terminate() override;



  protected:

    // Data members
    std::string m_svdRecoDigitsName; /**< SVDRecoDigit collection name */


    StoreArray<SVDRecoDigit>
    m_svdRecoDigits; /**< StoreArray for SVDRecoDigits */

    std::string m_occupancyOutputFile; /**< Output root filename */


    std::map<std::string, TH1F* > histMap; /**< map to store sensor histograms */
    int nEvents; /**< Event counter for scaling occupancy events */

  };
} // end namespace Belle2
