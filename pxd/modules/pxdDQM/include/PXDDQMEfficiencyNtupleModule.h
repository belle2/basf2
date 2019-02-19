/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Lueck, Ulf Stolzenberg, Benjamin Schwenker, Uwe Gebauer        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#pragma once

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <pxd/dataobjects/PXDCluster.h>

#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/ROIid.h>


//root stuff
#include "TString.h"
#include "TVector3.h"

#include "TFile.h"
#include "TNtuple.h"


namespace Belle2 {

  /**
   * Creates the basic histograms for PXD Efficiency DQM
   * Simplified and adopted version of the testbeam pxd efficiency module
   */
  class PXDDQMEfficiencyNtupleModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    PXDDQMEfficiencyNtupleModule();

  private:

    /**
     * main function which fills trees and histograms
     */
    void event() override final;

    /**
     * initializes the need store arrays, trees and histograms
     */
    void initialize() override final;

    /**
     * terminate , save tuple to file if needed
     */
    void terminate() override final;


  private:
    /** helper functions to do some of the calculations*/
    /* returns the space point in local coordinates where the track hits the sensor:
      sensorInfo: info of the sensor under investigation
      aTrack: the track to be tested
      isgood: flag which is false if some error occured (do not use the point if false)
      du and dv are the uncertainties in u and v on the sensor plane of the fit (local coordinates)
     */
    TVector3 getTrackInterSec(VXD::SensorInfoBase& pxdSensorInfo, const RecoTrack& aTrack, bool& isgood, double& du, double& dv);

    int findClosestCluster(VxdID& vxdid, TVector3 intersection);

    bool isCloseToBorder(int u, int v, int checkDistance);

    bool isDeadPixelClose(int u, int v, int checkDistance, VxdID& moduleID);

    //Require tracks going through ROIs
    bool m_requireROIs;

    //if true alignment will be used!
    bool m_useAlignment;

    bool m_maskDeadPixels;

    bool m_cutBorders;

    //the geometry
    VXD::GeoCache& m_vxdGeometry;

    std::string m_pxdClustersName;
    std::string m_tracksName;
    std::string m_ROIsName;

    StoreArray<PXDCluster> m_pxdclusters;
    StoreArray<RecoTrack> m_tracks;
    StoreArray<ROIid> m_ROIs;

    double m_pcut; // pValue-Cut for tracks
    double m_momCut; // Cut on fitted track momentum
    double m_pTCut; // Cut on fitted track pT
    unsigned int m_minSVDHits; // Required hits in SVD strips for tracks
    int m_maskedDistance; // Distance inside which no dead pixel or module border is allowed

    TFile* m_file{};
    TNtuple* m_tuple{};

  };
}
