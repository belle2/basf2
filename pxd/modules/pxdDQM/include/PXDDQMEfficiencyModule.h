/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Lueck, Ulf Stolzenberg, Benjamin Schwenker, Uwe Gebauer        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDDQMEFFICIENCYMODULE_H
#define PXDDQMEFFICIENCYMODULE_H

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <pxd/dataobjects/PXDCluster.h>

#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/ROIid.h>

//root stuff
#include "TTree.h"
#include "TString.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TVector3.h"


namespace Belle2 {

  /**
   * Creates the basic histograms for PXD Efficiency DQM
   * Simplified and adopted version of the testbeam pxd efficiency module
   */
  class PXDDQMEfficiencyModule : public HistoModule {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    PXDDQMEfficiencyModule();

    /**
     * destructor
     */
    virtual ~PXDDQMEfficiencyModule();

    /**
     * main function which fills trees and histograms
     */
    virtual void event();

    /**
     * initializes the need store arrays, trees and histograms
     */
    virtual void initialize();

    /**
     * actually defines the trees and histograms
     */
    virtual void defineHisto();

    /**
     * Do the initialization here
     */
    virtual void beginRun();



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


    //if true alignment will be used!
    bool m_useAlignment;

    //the geometry
    VXD::GeoCache& m_vxdGeometry;

    //Where to save the histograms too
    std::string m_histogramDirectoryName;

    std::string m_pxdClustersName;
    std::string m_tracksName;
    std::string m_ROIsName;

    int m_u_bins;
    int m_v_bins;

    StoreArray<PXDCluster> m_pxdclusters;
    StoreArray<RecoTrack> m_tracks;
    StoreArray<ROIid> m_ROIs;

    double m_distcut; //distance cut in cm!

    //Histograms to later determine efficiency
    std::map<VxdID, TH2D*> m_h_track_hits;
    std::map<VxdID, TH2D*> m_h_matched_cluster;
  };
}
#endif /* PXDDQMEFFICIENCYMODULE_H */
