/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Schnell, Christian Wessel                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/dataobjects/DATCONROI.h>
// #include <tracking/dataobjects/DATCONTrack.h>
#include <tracking/modules/DATCON/DATCONTrack.h>
#include <tracking/dataobjects/DATCONMPH.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/gfbfield/GFGeant4Field.h>
#include <tracking/dataobjects/ROIid.h>
#include <tracking/dataobjects/PXDIntercept.h>

#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <set>
#include <time.h>
#include <vector>
#include <unordered_map>
#include <type_traits>

#include <boost/array.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/tuple/tuple.hpp>

#include <framework/core/Module.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/dataobjects/RelationElement.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>

#include <geometry/bfieldmap/BFieldMap.h>
#include <geometry/GeometryManager.h>

#include <mdst/dataobjects/MCParticle.h>

#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/dataobjects/PXDSimHit.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/geometry/SensorInfo.h>

#include <root/TCanvas.h>
#include <root/TEfficiency.h>
#include <root/TFile.h>
#include <root/TF1.h>
#include <root/TH1D.h>
#include <root/TH1F.h>
#include <root/TH2D.h>
#include <root/TGeoMatrix.h>
#include <root/TGraph.h>
#include <root/TMath.h>
#include <root/TRandom.h>
#include <root/TTree.h>
#include <root/TVector2.h>
#include <root/TVector3.h>
#include <TDatabasePDG.h>
#include <TGeoManager.h>

#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/geometry/SensorInfo.h>
#include <svd/reconstruction/Sample.h>
#include <svd/simulation/SVDSignal.h>

#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/GeoCache.h>


namespace Belle2 {

  /** DATCONROICalculationModule class description */
  class DATCONROICalculationModule : public Module {
  public:
    /** Constructor.  */
    DATCONROICalculationModule();

    /** Standard Deconstructor */
    virtual ~DATCONROICalculationModule() = default;

    /** Initialize the module and check module parameters */
    virtual void initialize();
    /** Run tracking */
    virtual void event();

  protected:

    /** Members holding module parameters: */

    /** 1. Collections */
    /** Name of the PXDIntercepts array */
    std::string m_storeDATCONPXDInterceptsName;
    /** Name of the PXDIntercepts array */
    std::string m_storeDATCONMPHName;
    /** Name of the DATCONROIids array */
    std::string m_storeDATCONROIidName;

    StoreArray<DATCONMPH> m_storeDATCONMPHs;
    StoreArray<PXDIntercept> m_storeDATCONPXDIntercepts;
    StoreArray<ROIid> m_storeDATCONROIids;

    /** Continue ROI on neighbouring sensors? (default: true) */
    bool m_ContinueROIonNextSensor;
    /** Use fixed size ROIs, true = yes, no = false */
    bool m_useFixedSize;
    /** If using fixed ROI sizes: size of ROI in u-direction (default: 70) */
    int m_fixedSizeUCells;
    /** If using fixed ROI sizes: size of ROI in v-direction (default: 30) */
    int m_fixedSizeVCells;
    /** TODO */
    int m_multiplicativeU;
    /** TODO */
    int m_additiveU;
    /** TODO */
    int m_multiplicativeV;
    /** TODO */
    int m_additiveV;


  };//end class declaration
} // end namespace Belle2
