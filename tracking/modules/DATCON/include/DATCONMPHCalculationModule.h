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

#include <tracking/dataobjects/DATCONTrack.h>
// #include <tracking/modules/DATCON/DATCONTrack.h>
#include <tracking/dataobjects/DATCONMPH.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/PXDIntercept.h>

#include <cmath>
#include <vector>
#include <unordered_map>

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

#include <geometry/GeometryManager.h>

#include <mdst/dataobjects/MCParticle.h>

#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/dataobjects/PXDSimHit.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/geometry/SensorInfo.h>

#include <root/TGeoMatrix.h>
#include <root/TGraph.h>
#include <root/TMath.h>
#include <root/TRandom.h>
#include <root/TTree.h>
#include <root/TVector2.h>
#include <root/TVector3.h>
#include <TDatabasePDG.h>
#include <TGeoManager.h>

#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/GeoCache.h>


namespace Belle2 {

  /** DATCONMPHCalculationModule class description */
  class DATCONMPHCalculationModule : public Module {

  public:
    /** Constructor.  */
    DATCONMPHCalculationModule();

    /** Standard Deconstructor */
    virtual ~DATCONMPHCalculationModule() = default;

    /** Initialize the module and check module parameters */
    virtual void initialize();
    /** Run tracking */
    virtual void event();

  protected:

    /** Members holding module parameters: */

    /** 1. Collections */
    /** Name of the collection for the SVD Hough tracks */
    std::string m_storeDATCONTracksName;
    /** Name of the PXDIntercepts StoreArray */
    std::string m_storeDATCONPXDInterceptsName;
    /** Name of the DATCONMPH StoreArray */
    std::string m_storeDATCONMPHName;

    StoreArray<DATCONTrack>   m_storeDATCONTracks;
    StoreArray<PXDIntercept>  m_storeDATCONPXDIntercepts;
    StoreArray<DATCONMPH>     m_storeDATCONMPHs;

  };//end class declaration
} // end namespace Belle2
