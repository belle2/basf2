/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDOCCUPANCYMODULE_H
#define PXDOCCUPANCYMODULE_H

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <pxd/dataobjects/PXDCluster.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <rawdata/dataobjects/RawPXD.h>
#include <framework/dataobjects/EventMetaData.h>

#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>

#include <genfit/Track.h>

//root stuff
#include "TTree.h"
#include "TString.h"
#include "TVector3.h"


namespace Belle2 {
  /**
   * writes data to a root file which can be used for the estimation of the PXD occupancy
   *
   * see short description
   *
   */
  class PXDOccupancyModule : public HistoModule {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    PXDOccupancyModule();

    /**
    * destructor
    */
    virtual ~PXDOccupancyModule();

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



    /** helper functions to do some of the calculations*/
    /* returns the space point in local coordinates where the track hits the sensor:
      sensorInfo: info of the sensor under investigation
      aTrack: the track to be tested
      isgood: flag which is false if some error occured (do not use the point if false)
      du and dv are the uncertainties in u and v on the sensor plane of the fit (local coordinates)
     */
    TVector3 getTrackInterSec(VXD::SensorInfoBase& svdSensorInfo, const genfit::Track* aTrack, bool& isgood, double& du, double& dv);

    int findClosestTrack(VxdID& vxdid, TVector3 intersection);

    /// Get the DHH and HLT trigger number from RawPXD
    bool getTrigNr(RawPXD& px, unsigned int& innerDHH, unsigned int& outerHLT);
    /// Unpack DHE(C) frame in dataptr
    bool unpack_dhc_frame(void* data, unsigned int& innerDHH, unsigned int& outerHLT);


  private:
    //if true alignment will be used!
    bool m_useAlignment;

    double m_distcut; //distance cut in cm!

    //if true a tree with lots of info will be filled if false only the histograms are filled
    bool m_writeTree;

    //the geometry
    VXD::GeoCache& m_vxdGeometry;

    std::string m_eventmetadataname;
    std::string m_pxdclustersname;
    std::string m_pxddigitsname;
    std::string m_tracksname;
    StoreArray<PXDDigit> m_pxddigits;
    StoreArray<PXDCluster> m_pxdclusters;
    StoreArray<RawPXD> m_storeRaw;
    StoreObjPtr<EventMetaData> storeEventMetaData;

    //tree to store needed information
    TTree* m_tree;

    //tree to store needed information
    TTree* m_eventtree;

    /**
     * variables to be stored in the tree
     */

    //fitted positions in local coordinates
    double m_u_fit, m_v_fit, m_sigma_u_fit, m_sigma_v_fit;
    int m_layernumber, m_laddernumber, m_sensornumber, m_badcluster, m_ucell_fit, m_vcell_fit;

    //track quality indicators
    double m_fit_pValue, m_fit_mom, m_fit_theta, m_fit_phi, m_charge, m_charge_pdg;
    int m_fit_ndf, m_event, m_run, m_subrun, m_track_matched, m_nsvdtracks;

    // Cluster informatiom
    double m_u_clus, m_v_clus;
    int m_clus_charge, m_clus_seedcharge, m_clus_size, m_clus_usize, m_clus_vsize;


    int m_pxdTriggerNr, m_triggerNr;



  };
}

#endif /* PXDOccupancyMODULE_H */
