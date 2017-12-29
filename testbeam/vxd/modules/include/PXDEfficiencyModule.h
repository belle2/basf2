/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Lueck, Ulf Stolzenberg, Benjamin Schwenker        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDEFFICIENCYMODULE_H
#define PXDEFFICIENCYMODULE_H

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <pxd/dataobjects/PXDCluster.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <framework/dataobjects/EventMetaData.h>

#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>

#include <tracking/dataobjects/RecoTrack.h>

//root stuff
#include "TTree.h"
#include "TString.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TVector3.h"


namespace Belle2 {
  /**
   * writes data to a root file which can be used for the estimation of the PXD efficiency
   *
   * see short description
   *
   */
  class PXDEfficiencyModule : public HistoModule {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    PXDEfficiencyModule();

    /**
    * destructor
    */
    virtual ~PXDEfficiencyModule();

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



  private:
    /** helper functions to do some of the calculations*/
    /* returns the space point in local coordinates where the track hits the sensor:
      sensorInfo: info of the sensor under investigation
      aTrack: the track to be tested
      isgood: flag which is false if some error occured (do not use the point if false)
      du and dv are the uncertainties in u and v on the sensor plane of the fit (local coordinates)
     */
    TVector3 getTrackInterSec(VXD::SensorInfoBase& svdSensorInfo, const RecoTrack* aTrack, bool& isgood, double& du, double& dv);

    int findClosestDigit(VxdID& vxdid, TVector3 intersection);
    int findClosestCluster(VxdID& vxdid, TVector3 intersection);

    /*
      Writes the dummy values to all the member variables later in the output
      There should be a more elegant solution than this...
     */
    void dummyAllMaps();

    //if true alignment will be used!
    bool m_useAlignment;

    double m_distcut; //distance cut in cm!
    double m_otherdistcut; //distance cut for otherpxd-hit in cm!

    //if true a tree with lots of info will be filled if false only the histograms are filled
    bool m_writeTree;

    //the geometry
    VXD::GeoCache& m_vxdGeometry;

    std::string m_eventmetadataname;
    std::string m_pxdclustersname;
    std::string m_pxddigitsname;
    std::string m_tracksname;
    std::string m_ROIsName;
    StoreArray<PXDDigit> m_pxddigits;
    StoreArray<PXDCluster> m_pxdclusters;
    StoreObjPtr<EventMetaData> storeEventMetaData;
    //StoreArray<RecoTrack> m_tracks;

    //tree to store needed information
    TTree* m_tree;
    /**
     * variables to be stored in the tree
     */
    //fitted positions in local coordinates
    std::map<VxdID, double> m_u_fit;
    std::map<VxdID, double> m_v_fit;
    std::map<VxdID, double> m_u_clus;
    std::map<VxdID, double> m_v_clus;
    std::map<VxdID, int> m_ucell_clus;
    std::map<VxdID, int> m_vcell_clus;
    std::map<VxdID, double> m_u_digi;
    std::map<VxdID, double> m_v_digi;
    std::map<VxdID, double> m_ucell_digi;
    std::map<VxdID, double> m_vcell_digi;
    std::map<VxdID, double> m_sigma_u_fit;
    std::map<VxdID, double> m_sigma_v_fit;
    std::map<VxdID, int> m_ucell_fit;
    std::map<VxdID, int> m_vcell_fit;
    std::map<VxdID, int> m_digit_matched;
    std::map<VxdID, int> m_cluster_matched;
    std::map<VxdID, int> m_otherpxd_digit_matched;
    std::map<VxdID, int> m_otherpxd_cluster_matched;

    // Cluster informatiom
    std::map<VxdID, double> m_clus_charge;
    std::map<VxdID, double> m_clus_seedcharge;
    std::map<VxdID, int> m_clus_size;
    std::map<VxdID, int> m_clus_usize;
    std::map<VxdID, int> m_clus_vsize;

    //track quality indicators
    double m_fit_pValue, m_fit_mom, m_fit_theta, m_fit_phi, m_fit_chi2, m_fit_charge, m_fit_x, m_fit_y, m_fit_z;
    int m_fit_ndf, m_event, m_run, m_subrun;
    std::map<VxdID, TH2D*> m_h_tracksdigit;
    std::map<VxdID, TH2D*> m_h_trackscluster;
    std::map<VxdID, TH2D*> m_h_digits;
    std::map<VxdID, TH2D*> m_h_cluster;
    std::map<VxdID, TH2D*> m_h_tracksROI;
    std::map<VxdID, TH2D*> m_h_digitsROI;
    std::map<VxdID, TH2D*> m_h_clusterROI;

    std::map<VxdID, TH1D*> m_h_frame_nr;
    std::map<VxdID, TH2D*> m_h_broken_frame_pos;

    //ROI information
    std::map<VxdID, int> m_roi_number_of;
    std::map<VxdID, int> m_roi_minU;
    std::map<VxdID, int> m_roi_minV;
    std::map<VxdID, int> m_roi_maxU;
    std::map<VxdID, int> m_roi_maxV;
    std::map<VxdID, int> m_roi_widthU;
    std::map<VxdID, int> m_roi_widthV;
    std::map<VxdID, int> m_roi_centerU;
    std::map<VxdID, int> m_roi_centerV;
    std::map<VxdID, int> m_roi_area;
    std::map<VxdID, int> m_roi_fit_inside;
    std::map<VxdID, int> m_roi_clus_inside;
    std::map<VxdID, int> m_roi_digi_inside;
    std::map<VxdID, double> m_roi_u_residual;
    std::map<VxdID, double> m_roi_v_residual;
    std::map<VxdID, int> m_roi_ucell_residual;
    std::map<VxdID, int> m_roi_vcell_residual;

    std::map<VxdID, int> m_matched_frame;
    std::map<VxdID, int> m_hit_count;
  };
}

#endif /* PXDEFFICIENCYMODULE_H */
